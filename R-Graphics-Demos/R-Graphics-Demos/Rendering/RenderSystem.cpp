#include "pch.h"
#include "RenderSystem.h"
#include "FrameResource.h"

R::Rendering::RenderSystem::RenderSystem(const uint32_t width, const uint32_t height, const HWND windowHandle, ECS::World& world, Job::JobSystem& jobSystem)
	:
	m_renderContext(width, height, windowHandle, jobSystem.GetNumWorkers()),
	m_pJobSystem(&jobSystem),
	m_basePass(&m_renderContext, m_pJobSystem),
	m_beginFrame(m_renderContext.GetDevice(), L"BeginFrame"),
	m_endFrame(m_renderContext.GetDevice(), L"EndFrame")
{
	LogErrorIfFailed(m_beginFrame.GetCommandList()->Close());
	LogErrorIfFailed(m_endFrame.GetCommandList()->Close());

	ThreadRenderContext<1> temp(m_renderContext.GetDevice(), L"InitializeResources");
	// TODO: Init Passes
	m_basePass.Init(temp.GetCommandList());

	LogErrorIfFailed(temp.GetCommandList()->Close());
	ID3D12CommandList* ppCommandLists[] = { temp.GetCommandList() };
	m_renderContext.GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	m_renderContext.WaitForGPU();
}

R::Rendering::RenderSystem::~RenderSystem()
{
}


void R::Rendering::RenderSystem::Render()
{
	if (m_renderContext.GetCurrentFrameResource()->GetCount() == 0)
		return;
	m_renderContext.GetCurrentFrameResource()->Release(m_renderContext.GetFence());
	assert(m_renderContext.GetCurrentFrameResource()->GetState() == FrameResource::FrameResourceState::e_free);

	// ========== Begin Frame ==========
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_renderContext.GetRTVHeap()->GetCPUDescriptorHandleForHeapStart(),
		m_renderContext.GetFrameIndex(), m_renderContext.GetRTVDescriptorSize());
	{
		m_beginFrame.Reset(m_renderContext.GetFrameIndex());

		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderContext.GetRenderTarget(m_renderContext.GetFrameIndex()),
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		m_beginFrame.GetCommandList()->ResourceBarrier(1, &barrier);

		const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		m_beginFrame.GetCommandList()->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

		LogErrorIfFailed(m_beginFrame.GetCommandList()->Close());
		ID3D12CommandList* ppCommandLists[] = { m_beginFrame.GetCommandList() };
		m_renderContext.GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	}


	// ========== Passes ==========
	// Todo Move this to affinity based job if needed?
	for (uint32_t i = 0; i < m_pJobSystem->GetNumWorkers(); i++)
	{
		m_renderContext.GetThreadContext(i)->Reset(m_renderContext.GetFrameIndex());
		m_renderContext.GetThreadContext(i)->GetCommandList()->RSSetViewports(1, m_renderContext.GetViewPort());
		m_renderContext.GetThreadContext(i)->GetCommandList()->RSSetScissorRects(1, m_renderContext.GetScissorRect());
	}

	// TODO : Submit GPU Work
	// ...
	m_basePass.Update(m_renderContext.GetCurrentFrameResource(), &rtvHandle);
	m_basePass.WaitForCompletion();

	// ========== End Frame ==========
	for (uint32_t i = 0; i < m_pJobSystem->GetNumWorkers(); i++)
	{
		LogErrorIfFailed(m_renderContext.GetThreadContext(i)->GetCommandList()->Close());
	}
	m_renderContext.GetCommandQueue()->ExecuteCommandLists(static_cast<UINT>(m_pJobSystem->GetNumWorkers()), m_renderContext.GetCommandLists());

	{
		m_endFrame.Reset(m_renderContext.GetFrameIndex());
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_renderContext.GetRenderTarget(m_renderContext.GetFrameIndex()),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		m_endFrame.GetCommandList()->ResourceBarrier(1, &barrier);
		LogErrorIfFailed(m_endFrame.GetCommandList()->Close());
		ID3D12CommandList* ppCommandLists[] = { m_endFrame.GetCommandList() };
		m_renderContext.GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	}

	LogErrorIfFailed(m_renderContext.GetSwapChain()->Present(1, 0));

	m_renderContext.GetCurrentFrameResource()->Submit(m_renderContext.GetCommandQueue(), m_renderContext.GetFence(), m_renderContext.GetFrameNumber());
}


