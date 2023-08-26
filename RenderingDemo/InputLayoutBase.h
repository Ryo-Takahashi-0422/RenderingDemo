#pragma once

class InputLayoutBase
{
protected:
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout;

public:
	~InputLayoutBase();
	std::vector<D3D12_INPUT_ELEMENT_DESC> GetInputLayout();
	size_t GetInputSize();
};