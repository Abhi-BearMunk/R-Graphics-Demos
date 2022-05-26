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

		m_updateJobDatas[i].constData = &m_jobConstData;
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
	m_jobConstData.frameResource = renderContext->GetCurrentFrameResource();

	// Calculate View-Proj Mat
	Camera* cam = renderContext->GetCamera();
	//XMMATRIX vp = XMMatrixPerspectiveFovLH(cam->fov * 0.0174533f, renderContext->GetAspectRatio(), cam->nearPlane, cam->farPlane);
	XMVECTOR pos = XMLoadFloat3(&cam->position);
	XMVECTOR fwd = XMLoadFloat3(&cam->forward);
	XMVECTOR up = XMLoadFloat3(&cam->up);
	
	XMStoreFloat4x4(&m_jobConstData.matVP, XMMatrixMultiply(XMMatrixLookToLH(pos, fwd, up),
		XMMatrixPerspectiveFovLH(cam->fov * 0.0174533f, renderContext->GetAspectRatio(), cam->nearPlane, cam->farPlane)));

	// Populate Jobs
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
			m_updateJobDatas[count].startIndex = startIndex;
			m_updateJobDatas[count].batchSize = std::min(updateBatchSize, m_entities[i].entityCount - j);
			//JobFunc(&datas[count]);
			startIndex += m_updateJobDatas[count].batchSize;
			count++;
		}
	}

	// Launch Jobs
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
	JobConstData* constData = data->constData;
	FrameResource* frameResource = constData->frameResource;
	Renderable* renderable;

	XMFLOAT3 scale{ 1, 1, 1 };
	XMVECTOR sv = XMLoadFloat3(&scale);
	XMFLOAT3 pos{ 0, 0, 0 };
	XMVECTOR pv = XMLoadFloat3(&pos);
	XMFLOAT4 rot{ 0, 0, 0, 1 };
	XMVECTOR rv = XMLoadFloat4(&rot);
	XMVECTOR translation;
	XMMATRIX vp = XMLoadFloat4x4(&constData->matVP);
	for (uint32_t k = 0; k < data->batchSize; k++)
	{
		renderable = frameResource->GetRenderable(data->startIndex + k);
		translation = XMLoadFloat3(&data->pos[k]);
		//XMMATRIX m = XMMatrixAffineTransformation(sv, pv, rv, translation);
		XMStoreFloat4x4(&renderable->matrix, XMMatrixTranspose(XMMatrixMultiply(XMMatrixTranslationFromVector(XMLoadFloat3(&data->pos[k])), vp)));

	}
}
