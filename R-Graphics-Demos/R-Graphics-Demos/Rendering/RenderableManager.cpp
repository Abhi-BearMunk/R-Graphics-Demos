#include "pch.h"
#include "RenderableManager.h"

R::Rendering::RenderableManager::RenderableManager(ECS::World& world, Job::JobSystem& jobSystem)
	:m_pJobSystem(&jobSystem)
{
	world.InterestedIn<ECS::Pos, ECS::Rotation, ECS::Scale, Mesh>(m_entities);
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
	Camera* cam		= renderContext->GetCamera();
	XMVECTOR pos	= XMLoadFloat3(&cam->position);
	XMVECTOR fwd	= XMLoadFloat3(&cam->forward);
	XMVECTOR up		= XMLoadFloat3(&cam->up);
	XMMATRIX view	= XMMatrixLookToLH(pos, fwd, up);
	XMMATRIX proj	= XMMatrixPerspectiveFovLH(cam->fov * DegToRad, renderContext->GetAspectRatio(), cam->nearPlane, cam->farPlane);
	XMStoreFloat4x4(&m_jobConstData.viewProj, XMMatrixMultiply(view, proj));

	// Populate Jobs
	std::uint32_t updateBatchSize = 0;
	for (std::uint32_t i = 0; i < m_entities.size(); i++)
	{
		updateBatchSize += m_entities[i].entityCount;
	}
	updateBatchSize /= 64;
	updateBatchSize = std::max(1000u, updateBatchSize);
	std::uint32_t count = 0;
	std::uint32_t startIndex = 0;
	for (std::uint32_t i = 0; i < m_entities.size(); i++)
	{
		for (std::uint32_t j = 0; j < m_entities[i].entityCount; j += updateBatchSize)
		{
			m_updateJobDatas[count].pos = &reinterpret_cast<ECS::Pos*>(m_entities[i].ppComps[0])[j];
			m_updateJobDatas[count].rot = &reinterpret_cast<ECS::Rotation*>(m_entities[i].ppComps[1])[j];
			m_updateJobDatas[count].scale = &reinterpret_cast<ECS::Scale*>(m_entities[i].ppComps[2])[j];
			m_updateJobDatas[count].mesh = &reinterpret_cast<Mesh*>(m_entities[i].ppComps[3])[j];
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

void R::Rendering::RenderableManager::UpdateJobFunc(void* param, std::uint32_t tid)
{
	UpdateJobData* data = reinterpret_cast<UpdateJobData*>(param);
	JobConstData* constData = data->constData;
	FrameResource* frameResource = constData->frameResource;
	Renderable* renderable;

	XMVECTOR scale;
	XMVECTOR rotation;
	XMVECTOR translation;
	XMMATRIX vp = XMLoadFloat4x4(&constData->viewProj);
	XMMATRIX model;

	for (std::uint32_t k = 0; k < data->batchSize; k++)
	{
		renderable = frameResource->GetRenderable(data->startIndex + k);
		scale = XMLoadFloat3(&data->scale[k]);
		rotation = XMLoadFloat4(&data->rot[k]);
		translation = XMLoadFloat3(&data->pos[k]);
		model = XMMatrixAffineTransformation(scale, XMVectorZero(), rotation, translation);
		XMStoreFloat4x4(&renderable->matrix, XMMatrixTranspose(XMMatrixMultiply(model, vp)));
		renderable->textureID = data->mesh[k].textureIdStart;
	}
}
