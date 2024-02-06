#pragma once

class GraphicsPipelineSetting/* : public IGraphicsPipelineSetting*/
{
private:
	D3D12_INPUT_ELEMENT_DESC inputLayouts[10];
	VertexInputLayout* vertexInputLayout = nullptr;
	ComPtr<ID3D12PipelineState> _pipelineState = nullptr;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpipeLine = {};
	D3D12_RENDER_TARGET_BLEND_DESC renderTargetDesc = {};
	HRESULT result;
	int count = 0;
public:
	GraphicsPipelineSetting(VertexInputLayout* _vertexInputLayout);
	HRESULT CreateGPStateWrapper(ComPtr<ID3D12Device> _dev,
		SetRootSignature* setRootSignature, ComPtr<ID3D10Blob> _vsBlob, ComPtr<ID3D10Blob> _psBlob);

	void SetInputlayout(int i, D3D12_INPUT_ELEMENT_DESC inputLayout);
	D3D12_GRAPHICS_PIPELINE_STATE_DESC SetGPL(
		SetRootSignature* setRootSignature, ComPtr<ID3D10Blob> _vsBlob, ComPtr<ID3D10Blob> _psBlob);

	ComPtr<ID3D12PipelineState> GetPipelineState();
};