#include <stdafx.h>
#include <IGraphicsPipelineSetting.h>

IGraphicsPipelineSetting::IGraphicsPipelineSetting(InputLayoutBase* _vertexInputLayout)
{
	vertexInputLayout = _vertexInputLayout;
}

IGraphicsPipelineSetting::~IGraphicsPipelineSetting()
{
}