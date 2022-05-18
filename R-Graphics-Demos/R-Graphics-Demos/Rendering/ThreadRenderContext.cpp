#include "pch.h"
#include "ThreadRenderContext.h"

R::Rendering::ThreadRenderContext::ThreadRenderContext(ID3D12Device* device, ID3D12PipelineState* defaultPSO)
{
	for (size_t i = 0; i < FrameBuffersCount; i++)
	{
		LogErrorIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator[i])));
	}
	LogErrorIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator[0].Get(), defaultPSO, IID_PPV_ARGS(&m_commandList)));
}

R::Rendering::ThreadRenderContext::~ThreadRenderContext()
{
}

void R::Rendering::ThreadRenderContext::Reset(uint32_t index, ID3D12PipelineState* initialState = nullptr)
{
	LogErrorIfFailed(m_commandAllocator[index]->Reset());
	LogErrorIfFailed(m_commandList->Reset(m_commandAllocator[index].Get(), initialState));
}
