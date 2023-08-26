#pragma once

class ViewCreator
{
private:
	PMDMaterialInfo* pmdMaterialInfo = nullptr;
	BufferHeapCreator* bufferHeapCreator = nullptr;

	D3D12_VERTEX_BUFFER_VIEW vbView = {}; // Vertexビュー
	D3D12_INDEX_BUFFER_VIEW ibView = {}; // (Vertex)Indexビュー
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {}; // 行列用の定数ビュー詳細
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {}; // デプスステンシルビュー詳細
	D3D12_SHADER_RESOURCE_VIEW_DESC depthSRVDesc = {}; // 深度マップ用シェーダーリソースビュー詳細
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc4MaterialAndTextureAndSph = {}; // マテリアル情報、テクスチャ、sph用の定数ビュー詳細
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc4MaterialAndTextureAndSph = {}; // シェーダーリソースビュー詳細
	D3D12_RENDER_TARGET_VIEW_DESC multipassRTVDesc = {}; // マルチパス用レンダーターゲットビュー詳細
	D3D12_SHADER_RESOURCE_VIEW_DESC multipassSRVDesc = {}; // マルチパス用レンダーターゲットビュー詳細
	D3D12_CONSTANT_BUFFER_VIEW_DESC effectCBVDesc = {}; // 画面エフェクト用CBV詳細
	D3D12_SHADER_RESOURCE_VIEW_DESC normalMapSRVDesc = {}; // ノーマルマップ用SRV詳細
	D3D12_CPU_DESCRIPTOR_HANDLE basicDescHeapHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE handle4SRVMultipass;

	ComPtr<ID3D12Resource> whiteBuff = nullptr;
	ComPtr<ID3D12Resource> BlackBuff = nullptr;
	ComPtr<ID3D12Resource> grayTexBuff = nullptr;

	void SetCBVDesc4Matrix();
	void SetDSVDesc();
	void SetCBVDesc4MaterialAndTextureAndSph();
	void SetSRVDesc4MaterialAndTextureAndSph();
	void SetRTVDesc4Multipass();
	void SetSRVDesc4Multipass();

public:
	ViewCreator(PMDMaterialInfo* _pmdMaterialInfo, BufferHeapCreator* _bufferHeapCreator);
	void CreateCBV4Matrix(ComPtr<ID3D12Device> _dev); // 行列用cbvを生成 戻り値がvoidなのに注意
	void CreateCBV4ImguiPostSetting(ComPtr<ID3D12Device> _dev); // imgui PostSetting用
	void CreateDSVWrapper(ComPtr<ID3D12Device> _dev); //デプスステンシルビュー作成
	void CreateCBVSRV4MateriallTextureSph(ComPtr<ID3D12Device> _dev); // マテリアル、テクスチャ、sphファイル用のCBV,SRVをまとめて作成
	void CreateRTV4Multipasses(ComPtr<ID3D12Device> _dev); // まとめてマルチパス用RTV作成
	void CreateSRV4Multipasses(ComPtr<ID3D12Device> _dev); // まとめてマルチパス用SRV作成
	void CreateVertexBufferView();
	void CreateIndexBufferView();

	D3D12_VERTEX_BUFFER_VIEW* GetVbView();
	D3D12_INDEX_BUFFER_VIEW* GetIbView();
};