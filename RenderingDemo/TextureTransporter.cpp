#include <stdafx.h>
#include <TextureTransporter.h>

void TextureTransporter::TransportPMDMaterialTexture(
	ResourceManager* _resourceManager,
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
	// �e�N�X�`���p�]���I�u�W�F�N�g�̃��T�C�Y
	auto size = _resourceManager->GetMaterialAndTexturePath().size();
	size_t sourceSize = size * 4;
	source.resize(sourceSize);
	dest.resize(size);
	texBarriierDesc.resize(size);

	for (int count = 0; count < size; count++)
	{
		int sourceCount = count * 4;
		if (uploadBuff[sourceCount] == nullptr || readBuff[count] == nullptr) continue;

		source[sourceCount].pResource = uploadBuff[sourceCount].Get();
		source[sourceCount].Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		source[sourceCount].PlacedFootprint.Offset = 0;
		source[sourceCount].PlacedFootprint.Footprint.Width = metaData[sourceCount]->width;
		source[sourceCount].PlacedFootprint.Footprint.Height = metaData[sourceCount]->height;
		source[sourceCount].PlacedFootprint.Footprint.Depth = metaData[sourceCount]->depth;
		source[sourceCount].PlacedFootprint.Footprint.RowPitch =
			Utility::AlignmentSize(img[sourceCount]->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT); // R8G8B8A8:4bit * width�̒l��256�̔{���ł��邱��
		source[sourceCount].PlacedFootprint.Footprint.Format = img[sourceCount]->format;//metaData.format;

		source[sourceCount + 1].pResource = uploadBuff[sourceCount + 1].Get();
		source[sourceCount + 1].Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		source[sourceCount + 1].PlacedFootprint.Offset = 0;
		source[sourceCount + 1].PlacedFootprint.Footprint.Width = metaData[sourceCount + 1]->width;
		source[sourceCount + 1].PlacedFootprint.Footprint.Height = metaData[sourceCount + 1]->height;
		source[sourceCount + 1].PlacedFootprint.Footprint.Depth = metaData[sourceCount + 1]->depth;
		source[sourceCount + 1].PlacedFootprint.Footprint.RowPitch =
			Utility::AlignmentSize(img[sourceCount + 1]->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
		source[sourceCount + 1].PlacedFootprint.Footprint.Format = img[sourceCount + 1]->format;

		source[sourceCount + 2].pResource = uploadBuff[sourceCount + 1].Get();
		source[sourceCount + 2].Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		source[sourceCount + 2].PlacedFootprint.Offset = 0;
		source[sourceCount + 2].PlacedFootprint.Footprint.Width = metaData[sourceCount + 2]->width;
		source[sourceCount + 2].PlacedFootprint.Footprint.Height = metaData[sourceCount + 2]->height;
		source[sourceCount + 2].PlacedFootprint.Footprint.Depth = metaData[sourceCount + 2]->depth;
		source[sourceCount + 2].PlacedFootprint.Footprint.RowPitch =
			Utility::AlignmentSize(img[sourceCount + 2]->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
		source[sourceCount + 2].PlacedFootprint.Footprint.Format = img[sourceCount + 2]->format;

		source[sourceCount + 3].pResource = uploadBuff[sourceCount + 3].Get();
		source[sourceCount + 3].Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		source[sourceCount + 3].PlacedFootprint.Offset = 0;
		source[sourceCount + 3].PlacedFootprint.Footprint.Width = metaData[sourceCount + 3]->width;
		source[sourceCount + 3].PlacedFootprint.Footprint.Height = metaData[sourceCount + 3]->height;
		source[sourceCount + 3].PlacedFootprint.Footprint.Depth = metaData[sourceCount + 3]->depth;
		source[sourceCount + 3].PlacedFootprint.Footprint.RowPitch =
			Utility::AlignmentSize(img[sourceCount + 3]->rowPitch, D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);
		source[sourceCount + 3].PlacedFootprint.Footprint.Format = img[sourceCount + 3]->format;

		//�R�s�[��ݒ�
		dest[count].pResource = readBuff[count].Get();
		dest[count].Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dest[count].SubresourceIndex = 0;

		{
			_cmdList->CopyTextureRegion(&dest[count], 0, 0, 0, &source[sourceCount], nullptr);
			_cmdList->CopyTextureRegion(&dest[count], 0, 0, 0, &source[sourceCount + 1], nullptr);
			_cmdList->CopyTextureRegion(&dest[count], 0, 0, 0, &source[sourceCount + 2], nullptr);
			_cmdList->CopyTextureRegion(&dest[count], 0, 0, 0, &source[sourceCount + 3], nullptr);

			//�o���A�ݒ�...�����Ƃ��AStateAfter��...Generic_Read�Ȃǂɂ��Ă����s�\�B�����L�ڌ������炸�ڍוs���B
			texBarriierDesc[count].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			texBarriierDesc[count].Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			texBarriierDesc[count].Transition.pResource = readBuff[count].Get();
			texBarriierDesc[count].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			texBarriierDesc[count].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
			texBarriierDesc[count].Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

			_cmdList->ResourceBarrier(1, &texBarriierDesc[count]);
			_cmdList->Close();
			//�R�}���h���X�g�̎��s
			ID3D12CommandList* cmdlists[] = { _cmdList.Get() };
			_cmdQueue->ExecuteCommandLists(1, cmdlists);
			//�R�}���h���X�g���s�����������t�F���X���ʒm����܂őҋ@
			
			// ���s�������ؽĊ�����ɁA�w��̪ݽ�l��̪ݽ�ɏ������ނ悤�ݒ�B�����łͲݸ���Ă���fenceVal��̪ݽ�ɏ������ނ��ƂƂȂ�B
			_cmdQueue->Signal(_fence.Get(), ++_fenceVal);

			if (_fence->GetCompletedValue() != _fenceVal) // �t�F���X���ݒl��_fenceVal�����Ȃ�R�}���h���s�������Ȃ̂ňȉ��ҋ@����
			{
				auto event = CreateEvent(nullptr, false, false, nullptr);
				_fence->SetEventOnCompletion(_fenceVal, event); // �t�F���X���������ɒB������event����(�V�O�i����Ԃɂ���)
				WaitForSingleObject(event, INFINITE); // event���V�O�i����ԂɂȂ�܂őҋ@����
				CloseHandle(event); // event�n���h�������(�I��������)
			}
			_cmdAllocator->Reset();//�L���[���N���A
			_cmdList->Reset(_cmdAllocator.Get(), nullptr);
		}
	}
}

TextureTransporter::~TextureTransporter()
{
	resourceManager = nullptr;
}