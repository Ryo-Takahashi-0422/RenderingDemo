#pragma once

class ResourceManager
{
private:
	ComPtr<ID3D12Device> _dev = nullptr;
	FBXInfoManager* _fbxInfoManager = nullptr;

	ComPtr<ID3D12DescriptorHeap> dsvHeap = nullptr; // 深度ステンシルビュー用ディスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> rtvHeap = nullptr; // RTV用ディスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> srvHeap = nullptr; // SRV用ディスクリプタヒープ

	ComPtr<ID3D12Resource> vertBuff = nullptr; // 頂点用バッファ
	ComPtr<ID3D12Resource> idxBuff = nullptr; // 頂点インデックス用バッファ
	ComPtr<ID3D12Resource> depthBuff = nullptr; // デプスバッファー

public:
	ResourceManager(ComPtr<ID3D12Device> dev, FBXInfoManager* fbxInfoManager);
	HRESULT Init();
	
};

