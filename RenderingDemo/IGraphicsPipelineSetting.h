#pragma once

class IGraphicsPipelineSetting
{
protected:
	InputLayoutBase* vertexInputLayout = nullptr;
	ComPtr<ID3D12PipelineState> _pipelineState = nullptr;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeLine = {};
	D3D12_RENDER_TARGET_BLEND_DESC renderTargetDesc = {};
	HRESULT result;
	int count = 0;

//public:
	IGraphicsPipelineSetting(InputLayoutBase* _vertexInputLayout);
	~IGraphicsPipelineSetting();
	virtual HRESULT CreateGPStateWrapper(ComPtr<ID3D12Device> _dev,
		SetRootSignatureBase* setRootSignature, ComPtr<ID3D10Blob> _vsBlob, ComPtr<ID3D10Blob> _psBlob) = 0;

	virtual void SetInputlayout(int i, D3D12_INPUT_ELEMENT_DESC inputLayout) = 0;
	virtual D3D12_GRAPHICS_PIPELINE_STATE_DESC SetGPL(
		SetRootSignatureBase* setRootSignature, ComPtr<ID3D10Blob> _vsBlob, ComPtr<ID3D10Blob> _psBlob) = 0;

	virtual ComPtr<ID3D12PipelineState> GetPipelineState() = 0;
};