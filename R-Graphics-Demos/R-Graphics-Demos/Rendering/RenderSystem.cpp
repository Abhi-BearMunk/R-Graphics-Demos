#include "pch.h"
#include "RenderSystem.h"
#include "FrameResource.h"

R::Rendering::RenderSystem::RenderSystem(const uint32_t width, const uint32_t height, const HWND windowHandle, ECS::World& world, Job::JobSystem& jobSystem)
	:
	m_globalRenderContext(width, height, windowHandle), 
	m_pJobSystem(&jobSystem),
	m_threadRenderContexts(reinterpret_cast<ThreadRenderContext<FrameBuffersCount>*>(operator new[](sizeof(ThreadRenderContext<FrameBuffersCount>) * jobSystem.GetNumWorkers()))),
	m_threadCommandLists(new ID3D12GraphicsCommandList*[jobSystem.GetNumWorkers()]),
	m_currentFrameResourceIndex(1),
	m_basePass(&m_globalRenderContext, m_threadRenderContexts, m_pJobSystem),
	m_beginFrame(m_globalRenderContext.GetDevice(), L"BeginFrame"),
	m_endFrame(m_globalRenderContext.GetDevice(), L"EndFrame")
{
	for (size_t i = 0; i < jobSystem.GetNumWorkers(); i++)
	{
		new (&m_threadRenderContexts[i]) ThreadRenderContext<FrameBuffersCount>(m_globalRenderContext.GetDevice(), L"ThreadContext_" + std::to_wstring(i));
		LogErrorIfFailed(m_threadRenderContexts[i].GetCommandList()->Close());
		m_threadCommandLists[i] = m_threadRenderContexts[i].GetCommandList();
	}
	LogErrorIfFailed(m_beginFrame.GetCommandList()->Close());
	LogErrorIfFailed(m_endFrame.GetCommandList()->Close());

	world.InterestedIn<ECS::Pos>(m_entities);
	for (int i = 0; i < ECS::MAX_ENTITIES_PER_ARCHETYPE; i++)
	{
		m_updateJobDescs[i].jobFunc = &RenderSystem::UpdateJobFunc;
		m_updateJobDescs[i].param = &m_updateJobDatas[i];
		m_updateJobDescs[i].pCounter = &m_updateCounter;
	}

	ThreadRenderContext<1> temp(m_globalRenderContext.GetDevice(), L"InitializeResources");
	// TODO: Init Passes
	// ...
	m_basePass.Init(temp.GetCommandList());

	LogErrorIfFailed(temp.GetCommandList()->Close());
	ID3D12CommandList* ppCommandLists[] = { temp.GetCommandList() };
	m_globalRenderContext.GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	m_globalRenderContext.WaitForGPU();
}

R::Rendering::RenderSystem::~RenderSystem()
{
	delete[] m_updateJobDescs;
	delete[] m_updateJobDatas;
	for (size_t i = 0; i < m_pJobSystem->GetNumWorkers(); i++)
	{
		m_threadRenderContexts[i].~ThreadRenderContext();
	}
	operator delete[](m_threadRenderContexts);
}

void R::Rendering::RenderSystem::Update(const float& dt)
{
	// Increment the frame resource index
	m_currentFrameResourceIndex = (m_currentFrameResourceIndex + 1) % FrameBuffersCount;
	uint32_t updateBatchSize = 0;
	for (uint32_t i = 0; i < m_entities.size(); i++)
	{
		updateBatchSize += m_entities[i].entityCount;
	}
	updateBatchSize /= 64;
	updateBatchSize = std::max(1000u, updateBatchSize);
	uint32_t count = 0;
	uint32_t startIndex = 0;
	for (uint32_t i = 0; i < m_entities.size(); i++)
	{
		for (uint32_t j = 0; j < m_entities[i].entityCount; j += updateBatchSize)
		{
			m_updateJobDatas[count].pos = &reinterpret_cast<ECS::Pos*>(m_entities[i].ppComps[0])[j];
			m_updateJobDatas[count].frameResource = &m_frameResources[m_currentFrameResourceIndex];
			m_updateJobDatas[count].startIndex = startIndex;
			m_updateJobDatas[count].batchSize = std::min(updateBatchSize, m_entities[i].entityCount - j);
			//JobFunc(&datas[count]);
			startIndex += m_updateJobDatas[count].batchSize;
			count++;
		}
	}
	m_frameResources[m_currentFrameResourceIndex].SetCount(startIndex);
	// All update jobs from previous frame should have finished
	assert(m_updateCounter.counter == 0);
	m_updateCounter.counter = count; 
	m_pJobSystem->KickJobsWithPriority(m_updateJobDescs, count);
}

void R::Rendering::RenderSystem::WaitForUpdate()
{
	m_pJobSystem->WaitForCounter(&m_updateCounter);
}

void R::Rendering::RenderSystem::Render()
{
	if (m_frameResources[m_currentFrameResourceIndex].GetCount() == 0)
		return;
	m_frameResources[m_currentFrameResourceIndex].Release(m_globalRenderContext.GetFence());
	assert(m_frameResources[m_currentFrameResourceIndex].GetState() == FrameResource::FrameResourceState::e_free);

	// ========== Begin Frame ==========
	{
		m_beginFrame.Reset(m_currentFrameResourceIndex);

		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_globalRenderContext.GetRenderTarget(m_currentFrameResourceIndex),
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		m_beginFrame.GetCommandList()->ResourceBarrier(1, &barrier);

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_globalRenderContext.GetRTVHeap()->GetCPUDescriptorHandleForHeapStart(),
			m_currentFrameResourceIndex, m_globalRenderContext.GetRTVDescriptoSize());
		const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		m_beginFrame.GetCommandList()->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

		LogErrorIfFailed(m_beginFrame.GetCommandList()->Close());
		ID3D12CommandList* ppCommandLists[] = { m_beginFrame.GetCommandList() };
		m_globalRenderContext.GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	}


	// ========== Passes ==========
	// Todo Move this to affinity based job if needed?
	for (int i = 0; i < m_pJobSystem->GetNumWorkers(); i++)
	{
		m_threadRenderContexts[i].Reset(m_currentFrameResourceIndex);
		m_threadRenderContexts[i].GetCommandList()->RSSetViewports(1, m_globalRenderContext.GetViewPort());
		m_threadRenderContexts[i].GetCommandList()->RSSetScissorRects(1, m_globalRenderContext.GetScissorRect());
	}

	// TODO : Submit GPU Work
	// ...
	m_basePass.Update(&m_frameResources[m_currentFrameResourceIndex], m_currentFrameResourceIndex);
	m_basePass.WaitForCompletion();

	// ========== End Frame ==========
	for (uint32_t i = 0; i < m_pJobSystem->GetNumWorkers(); i++)
	{
		LogErrorIfFailed(m_threadCommandLists[i]->Close());
	}
	m_globalRenderContext.GetCommandQueue()->ExecuteCommandLists(static_cast<UINT>(m_pJobSystem->GetNumWorkers()), CommandListCast(m_threadCommandLists));

	{
		m_endFrame.Reset(m_currentFrameResourceIndex);
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_globalRenderContext.GetRenderTarget(m_currentFrameResourceIndex),
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		m_endFrame.GetCommandList()->ResourceBarrier(1, &barrier);
		LogErrorIfFailed(m_endFrame.GetCommandList()->Close());
		ID3D12CommandList* ppCommandLists[] = { m_endFrame.GetCommandList() };
		m_globalRenderContext.GetCommandQueue()->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	}

	LogErrorIfFailed(m_globalRenderContext.GetSwapChain()->Present(1, 0));

	m_frameResources[m_currentFrameResourceIndex].Submit(m_globalRenderContext.GetCommandQueue(), m_globalRenderContext.GetFence(), m_globalRenderContext.GetFrameNumber());
}

void R::Rendering::RenderSystem::UpdateJobFunc(void* param, uint32_t tid)
{
	UpdateJobData* data = reinterpret_cast<UpdateJobData*>(param);
	Renderable* renderable;
	for (uint32_t k = 0; k < data->batchSize; k++)
	{
		renderable = data->frameResource->GetRenderable(data->startIndex + k);
		renderable->x = data->pos[k].x;
		renderable->y = data->pos[k].y;
	}
}


