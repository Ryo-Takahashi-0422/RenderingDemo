#pragma once

class VertexInputLayout// : public  InputLayoutBase
{
private:
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout;

public:
	VertexInputLayout();
	~VertexInputLayout();
	std::vector<D3D12_INPUT_ELEMENT_DESC> GetInputLayout();
	size_t GetInputSize();
	//std::vector<D3D12_INPUT_ELEMENT_DESC> GetInputLayout();
	//size_t GetInputSize();
};