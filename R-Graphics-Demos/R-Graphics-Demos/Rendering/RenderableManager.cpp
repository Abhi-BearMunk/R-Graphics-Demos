#include "pch.h"
#include "RenderableManager.h"

R::Rendering::RenderableManager::RenderableManager(ECS::World& world, Job::JobSystem& jobSystem)
	:m_pJobSystem(&jobSystem)
{
	world.InterestedIn<ECS::Pos>(m_entities);
	for (int i = 0; i < ECS::MAX_ENTITIES_PER_ARCHETYPE; i++)
	{
		m_updateJobDescs[i].jobFunc = &RenderableManager::UpdateJobFunc;
		m_updateJobDescs[i].param = &m_updateJobDatas[i];
		m_updateJobDescs[i].pCounter = &m_updateCounter;
	}
}

R::Rendering::RenderableManager::~RenderableManager()
{
	delete[] m_updateJobDescs;
	delete[] m_updateJobDatas;
}

void R::Rendering::RenderableManager::Update(RenderContext* renderContext)
{
	// Increment the frame resource index
	renderContext->AdvanceFrame();
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
			m_updateJobDatas[count].frameResource = renderContext->GetCurrentFrameResource();
			m_updateJobDatas[count].startIndex = startIndex;
			m_updateJobDatas[count].batchSize = std::min(updateBatchSize, m_entities[i].entityCount - j);
			//JobFunc(&datas[count]);
			startIndex += m_updateJobDatas[count].batchSize;
			count++;
		}
	}
	renderContext->GetCurrentFrameResource()->SetCount(startIndex);
	// All update jobs from previous frame should have finished
	assert(m_updateCounter.counter == 0);
	m_updateCounter.counter = count;
	m_pJobSystem->KickJobsWithPriority(m_updateJobDescs, count);
}

void R::Rendering::RenderableManager::WaitForUpdate()
{
	m_pJobSystem->WaitForCounter(&m_updateCounter);
}

void R::Rendering::RenderableManager::UpdateJobFunc(void* param, uint32_t tid)
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
