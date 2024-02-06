#pragma once

class PeraLayout// : public InputLayoutBase
{
private:
	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout;

public:
	PeraLayout();
	~PeraLayout();
	std::vector<D3D12_INPUT_ELEMENT_DESC> GetInputLayout();
	size_t GetInputSize();
};