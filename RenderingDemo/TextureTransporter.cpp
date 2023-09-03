#include <stdafx.h>
#include <TextureTransporter.h>

TextureTransporter::TextureTransporter(FBXInfoManager* _fbxInfoManager)
{
	fbxInfoManager = new FBXInfoManager;
	fbxInfoManager = _fbxInfoManager;
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
	std::vector<ComPtr<ID3D12Resource>> readBuff
	)
{
	// テクスチャ用転送オブジェクトのリサイズ
	auto size = fbxInfoManager->GetMaterialAndTexturePath().size();
	source.resize(size);
	dest.resize(size);
	texBarriierDesc.resize(size);

	for (int count = 0; count < size; count++)
	{
		if (uploadBuff[count] == nullptr || readBuff[count] == nullptr) continue;

		source[count].pResource = uploadBuff[count].Get();
		source[count].Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		source[count].PlacedFootprint.Offset = 0;
		source[count].PlacedFootprint.Footprint.Width = metaData[count]->width;
		source[count].PlacedFootprint.Footprint.Height = metaData[count]->height;
		source[count].PlacedFootprint.Footprint.Depth = metaData[count]->depth;
		source[count].PlacedFootprint.Footprint.RowPitch =
			Utility::AlignmentSize(img[count]->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT); // R8G8B8A8:4bit * widthの値は256の倍数であること
		source[count].PlacedFootprint.Footprint.Format = img[count]->format;//metaData.format;

		//コピー先設定
		dest[count].pResource = readBuff[count].Get();
		dest[count].Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dest[count].SubresourceIndex = 0;

		{
			_cmdList->CopyTextureRegion(&dest[count], 0, 0, 0, &source[count], nullptr);

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