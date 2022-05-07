#include "pch.h"
#include "ThreadRenderContext.h"

R::Rendering::ThreadRenderContext::ThreadRenderContext(ID3D12Device* device, ID3D12PipelineState* defaultPSO)
{
	LogErrorIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));
	LogErrorIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), defaultPSO, IID_PPV_ARGS(&m_commandList)));
}

R::Rendering::ThreadRenderContext::~ThreadRenderContext()
{
}
