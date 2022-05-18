#include "pch.h"
#include "RenderSystem.h"
#include "FrameResource.h"

R::Rendering::RenderSystem::RenderSystem(const uint32_t width, const uint32_t height, const HWND windowHandle, ECS::World& world, Job::JobSystem& jobSystem)
	:
	m_globalRenderContext(width, height, windowHandle), 
	m_pJobSystem(&jobSystem),
	m_threadRenderContexts(reinterpret_cast<ThreadRenderContext*>(malloc(sizeof(ThreadRenderContext) * jobSystem.GetNumWorkers()))),
	m_currentFrameResourceIndex(0)
{
	for (size_t i = 0; i < jobSystem.GetNumWorkers(); i++)
	{
		new (&m_threadRenderContexts[i]) ThreadRenderContext(m_globalRenderContext.GetDevice());
	}
	world.InterestedIn<ECS::Pos>(m_entities);
	for (int i = 0; i < ECS::MAX_ENTITIES_PER_ARCHETYPE; i++)
	{
		m_updateJobDescs[i].jobFunc = &RenderSystem::UpdateJobFunc;
		m_updateJobDescs[i].param = &m_updateJobDatas[i];
		m_updateJobDescs[i].pCounter = &m_updateCounter;
	}
}

R::Rendering::RenderSystem::~RenderSystem()
{
	free(m_threadRenderContexts);
}

void R::Rendering::RenderSystem::Update(const float& dt)
{
	m_frameResources[m_currentFrameResourceIndex].Release(m_globalRenderContext.GetFence());
	uint32_t updateBatchSize = 0;
	for (uint32_t i = 0; i < m_entities.size(); i++)
	{
		updateBatchSize += m_entities[i].entityCount;
	}
	updateBatchSize /= 64;
	uint32_t count = 0;
	uint32_t startIndex = 0;
	for (uint32_t i = 0; i < m_entities.size(); i++)
	{
		for (uint32_t j = 0; j < m_entities[i].entityCount; j += updateBatchSize)
		{
			m_updateJobDatas[count].pos = &reinterpret_cast<ECS::Pos*>(m_entities[i].ppComps[0])[j];
			m_updateJobDatas[count].frameResource = &m_frameResources[m_currentFrameResourceIndex];
			m_updateJobDatas[count].startIndex = startIndex;
			m_updateJobDatas[count].batchSize = std::min(updateBatchSize, m_entities[i].entityCount - j - 1);
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
	// Increment the frame resource index
	m_currentFrameResourceIndex = (m_currentFrameResourceIndex + 1) % FrameBuffersCount;
}

void R::Rendering::RenderSystem::WaitForUpdate()
{
	m_pJobSystem->WaitForCounter(&m_updateCounter);
}

void R::Rendering::RenderSystem::Render()
{
	assert(m_frameResources[m_currentFrameResourceIndex].GetState() == FrameResource::FrameResourceState::e_free);
	// Todo Move this to affinity based job if needed?
	for (size_t i = 0; i < m_pJobSystem->GetNumWorkers(); i++)
	{
		m_threadRenderContexts[i].Reset(m_currentFrameResourceIndex);
	}
	uint32_t renderBatchSize = m_frameResources[m_currentFrameResourceIndex].GetCount() / 64;

}

void R::Rendering::RenderSystem::WaitForRender()
{
}

void R::Rendering::RenderSystem::UpdateJobFunc(void* param, uint32_t tid)
{
	UpdateJobData* data = reinterpret_cast<UpdateJobData*>(param);
	Renderable* renderable;
	for (int k = 0; k < data->batchSize; k++)
	{
		renderable = data->frameResource->GetRenderable(data->startIndex + k);
		renderable->x = data->pos->x;
		renderable->y = data->pos->y;
	}
}

void R::Rendering::RenderSystem::RenderJobFunc(void* param, uint32_t tid)
{
}


