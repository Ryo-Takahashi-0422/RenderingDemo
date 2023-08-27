#include <stdafx.h>
#include <VertexInputLayout.h>

VertexInputLayout::VertexInputLayout()
{
	inputLayout =
	{
		//座標
		{
			"POSITION",
			0, // 同じセマンティクスに対するインデックス
			DXGI_FORMAT_R32G32B32_FLOAT,
			0, // スロットインデックス
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0 // 一度に描画するインスタンス数
		}//,

		////法線ベクトル
		//{
		//	"NORMAL",
		//	0,
		//	DXGI_FORMAT_R32G32B32_FLOAT,
		//	0,
		//	D3D12_APPEND_ALIGNED_ELEMENT,
		//	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
		//	0
		//},

		////uv
		//{
		//	"TEXCOORD",
		//	0,
		//	DXGI_FORMAT_R32G32_FLOAT,
		//	0, // スロットインデックス
		//	D3D12_APPEND_ALIGNED_ELEMENT,
		//	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
		//	0
		//},

		////ボーン番号
		//{
		//	"BONE_NO",
		//	0,
		//	DXGI_FORMAT_R16G16_UINT, // bone[0], bone[1]
		//	0,
		//	D3D12_APPEND_ALIGNED_ELEMENT,
		//	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
		//	0
		//},

		////ボーンウェイト
		//{
		//	"WEIGHT",
		//	0,
		//	DXGI_FORMAT_R8_UINT,
		//	0,
		//	D3D12_APPEND_ALIGNED_ELEMENT,
		//	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
		//	0
		//},

		////エッジフラグ
		//{
		//	"EDGE_FLG",
		//	0,
		//	DXGI_FORMAT_R8_UINT,
		//	0,
		//	D3D12_APPEND_ALIGNED_ELEMENT,
		//	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
		//	0
		//}
	};
}

VertexInputLayout::~VertexInputLayout()
{
}
//
//std::vector<D3D12_INPUT_ELEMENT_DESC> VertexInputLayout::GetInputLayout()
//{
//	return inputLayout;
//}
//
//size_t VertexInputLayout::GetInputSize()
//{
//	return inputLayout.size();
//}