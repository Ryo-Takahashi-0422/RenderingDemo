#pragma once

class ResourceManager
{
private:
	ComPtr<ID3D12Device> _dev = nullptr;
	FBXInfoManager* _fbxInfoManager = nullptr;

	ComPtr<ID3D12DescriptorHeap> dsvHeap = nullptr; // �[�x�X�e���V���r���[�p�f�B�X�N���v�^�q�[�v
	ComPtr<ID3D12DescriptorHeap> rtvHeap = nullptr; // RTV�p�f�B�X�N���v�^�q�[�v
	ComPtr<ID3D12DescriptorHeap> srvHeap = nullptr; // SRV�p�f�B�X�N���v�^�q�[�v

	ComPtr<ID3D12Resource> vertBuff = nullptr; // ���_�p�o�b�t�@
	ComPtr<ID3D12Resource> idxBuff = nullptr; // ���_�C���f�b�N�X�p�o�b�t�@
	ComPtr<ID3D12Resource> depthBuff = nullptr; // �f�v�X�o�b�t�@�[

public:
	ResourceManager(ComPtr<ID3D12Device> dev, FBXInfoManager* fbxInfoManager);
	HRESULT Init();
	
};

