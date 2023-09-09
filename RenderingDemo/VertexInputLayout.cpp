#include <stdafx.h>
#include <VertexInputLayout.h>

VertexInputLayout::VertexInputLayout()
{
	inputLayout =
	{
		//���W
		{
			"POSITION",
			0, // �����Z�}���e�B�N�X�ɑ΂���C���f�b�N�X
			DXGI_FORMAT_R32G32B32_FLOAT,
			0, // �X���b�g�C���f�b�N�X
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0 // ��x�ɕ`�悷��C���X�^���X��
		},

		//�@���x�N�g��
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
			0, // �X���b�g�C���f�b�N�X
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},

		//�{�[���ԍ�1�Z�b�g��
		{
			"BONE_NO_ZeroToTwo",
			0,
			DXGI_FORMAT_R32G32B32_UINT, // unsigned shourt bone[0]-bone[2]
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},

		//�{�[���ԍ�2�Z�b�g��
		{
			"BONE_NO_ThreeToFive",
			0,
			DXGI_FORMAT_R32G32B32_UINT, // unsigned shourt bone[3]-bone[5]
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},

		//�{�[���E�F�C�g1�Z�b�g��
		{
			"WEIGHT_ZeroToTwo",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT, // float weight[0] - [2]
			0,
			D3D12_APPEND_ALIGNED_ELEMENT,
			D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			0
		},

		//�{�[���E�F�C�g2�Z�b�g��
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