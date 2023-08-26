#include <stdafx.h>
#include <TextureTransporter.h>

TextureTransporter::TextureTransporter(PMDMaterialInfo* _pmdMaterialInfo, BufferHeapCreator* _bufferHeapCreator)
{
	pmdMaterialInfo = new PMDMaterialInfo;
	pmdMaterialInfo = _pmdMaterialInfo;

	bufferHeapCreator = _bufferHeapCreator;
}

void TextureTransporter::TransportPMDMaterialTexture(
	ComPtr<ID3D12GraphicsCommandList> _cmdList,
	ComPtr<ID3D12CommandAllocator> _cmdAllocator,
	ComPtr<ID3D12CommandQueue> _cmdQueue,
	std::vector<DirectX::TexMetadata*> metaData,
	std::vector<DirectX::Image*> img,
	ComPtr<ID3D12Fence> _fence,
	UINT64& _fenceVal,
	std::vector<ComPtr<ID3D12Resource>> uploadBuff,
	std::vector<ComPtr<ID3D12Resource>> readBuff,
	unsigned int itCount)
{
	// テクスチャ用転送オブジェクトのリサイズ
	pmdSource.resize(pmdMaterialInfo->materialNum);
	pmdDestination.resize(pmdMaterialInfo->materialNum);
	texBarriierDesc.resize(pmdMaterialInfo->materialNum);

	for (int count = 0; count < itCount; count++)
	{
		if (uploadBuff[count] == nullptr || readBuff[count] == nullptr) continue;

		pmdSource[count].pResource = uploadBuff[count].Get();
		pmdSource[count].Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		pmdSource[count].PlacedFootprint.Offset = 0;
		pmdSource[count].PlacedFootprint.Footprint.Width = metaData[count]->width;
		pmdSource[count].PlacedFootprint.Footprint.Height = metaData[count]->height;
		pmdSource[count].PlacedFootprint.Footprint.Depth = metaData[count]->depth;
		pmdSource[count].PlacedFootprint.Footprint.RowPitch =
			Utility::AlignmentSize(img[count]->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT); // R8G8B8A8:4bit * widthの値は256の倍数であること
		pmdSource[count].PlacedFootprint.Footprint.Format = img[count]->format;//metaData.format;

		//コピー先設定
		pmdDestination[count].pResource = readBuff[count].Get();
		pmdDestination[count].Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		pmdDestination[count].SubresourceIndex = 0;

		{
			_cmdList->CopyTextureRegion(&pmdDestination[count], 0, 0, 0, &pmdSource[count], nullptr);

			//バリア設定...せずとも、StateAfterを...Generic_Readなどにしても実行可能。公式記載見当たらず詳細不明。
			texBarriierDesc[count].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			texBarriierDesc[count].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			texBarriierDesc[count].Transition.pResource = readBuff[count].Get();
			texBarriierDesc[count].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			texBarriierDesc[count].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
			texBarriierDesc[count].Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

			_cmdList->ResourceBarrier(1, &texBarriierDesc[count]);
			_cmdList->Close();
			//コマンドリストの実行
			ID3D12CommandList* cmdlists[] = { _cmdList.Get() };
			_cmdQueue->ExecuteCommandLists(1, cmdlists);
			//コマンドリスト実行完了したかフェンスが通知するまで待機
			
			// 実行中ｺﾏﾝﾄﾞﾘｽﾄ完了後に、指定ﾌｪﾝｽ値をﾌｪﾝｽに書き込むよう設定。ここではｲﾝｸﾘﾒﾝﾄしたfenceValをﾌｪﾝｽに書き込むこととなる。
			_cmdQueue->Signal(_fence.Get(), ++_fenceVal);

			if (_fence->GetCompletedValue() != _fenceVal) // フェンス現在値が_fenceVal未満ならコマンド実行未完了なので以下待機処理
			{
				auto event = CreateEvent(nullptr, false, false, nullptr);
				_fence->SetEventOnCompletion(_fenceVal, event); // フェンスが第一引数に達したらevent発火(シグナル状態にする)
				WaitForSingleObject(event, INFINITE); // eventがシグナル状態になるまで待機する
				CloseHandle(event); // eventハンドルを閉じる(終了させる)
			}
			_cmdAllocator->Reset();//キューをクリア
			_cmdList->Reset(_cmdAllocator.Get(), nullptr);
		}
	}
}