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
		},

		//法線ベクトル
		{
			"NORMAL_Vertex",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},

		//uv
		{
			"TEXCOORD",
			0,
			DXGI_FORMAT_R32G32_FLOAT,
			0, // スロットインデックス
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},

		//ボーン番号1セット目
		{
			"BONE_NO_ZeroToTwo",
			0,
			DXGI_FORMAT_R32G32B32_UINT, // unsigned shourt bone[0]-bone[2]
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},

		//ボーン番号2セット目
		{
			"BONE_NO_ThreeToFive",
			0,
			DXGI_FORMAT_R32G32B32_UINT, // unsigned shourt bone[3]-bone[5]
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},

		//ボーンウェイト1セット目
		{
			"WEIGHT_ZeroToTwo",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT, // float weight[0] - [2]
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},

		//ボーンウェイト2セット目
		{
			"WEIGHT_ThreeToFive",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT, // float weight[3] - [5]
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},

		//Tangent
		{
			"TANGENT",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},

		//Binormal
		{
			"BINORMAL",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},

		//vNormal
		{
			"NORMAL",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		}

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