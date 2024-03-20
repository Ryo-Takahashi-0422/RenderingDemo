#pragma once

struct PeraVertex
{
	XMFLOAT3 pos;
	XMFLOAT2 uv;
};

class PeraPolygon
{
private:
	PeraVertex pv[4] =
	{
		{{-1.0f,-1.0f,0.1f},{0.0f,1.0f}}, // 左下
		{{-1.0f,1.0f,0.1f},{0.0f,0.0f}}, // 左上
	    {{1.0f,-1.0f,0.1f},{1.0f,1.0f}}, // 右下
	    {{1.0f,1.0f,0.1f},{1.0f,0.0f}} // 右上
	};

	PeraVertex* mappedPera = nullptr;
	ComPtr<ID3D12Resource> peraBuff = nullptr; // ペラポリゴン用バッファ
	D3D12_VERTEX_BUFFER_VIEW peraVBV = {}; // ペラポリゴン用頂点ビュー

public:
	void CreatePeraView(ComPtr<ID3D12Device> _dev);
	~PeraPolygon();
	D3D12_VERTEX_BUFFER_VIEW* GetVBView();
};