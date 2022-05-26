#include "pch.h"
#include "MoveSystem.h"
#include <algorithm>

R::Test::MoveSystem::MoveSystem(ECS::World& world, Job::JobSystem& jobSystem)
	:pJobSystem(&jobSystem), moveBatchSize(0)
{
	world.InterestedIn<ECS::Pos, Test::Velocity>(entities);
	for (uint32_t i = 0; i < entities.size(); i++)
	{
		moveBatchSize += entities[i].entityCount;
	}
	moveBatchSize /= 64;
	moveBatchSize = std::max(moveBatchSize, 1000u);
	for (int i = 0; i < ECS::MAX_ENTITIES_PER_ARCHETYPE; i++)
	{
		jobDescs[i].jobFunc = &MoveSystem::JobFunc;
		jobDescs[i].param = &datas[i];
		jobDescs[i].pCounter = &moveCounter;
	}
}

R::Test::MoveSystem::~MoveSystem()
{
	delete[] datas;
	delete[] jobDescs;
}

void R::Test::MoveSystem::Update(const float& dt)
{
	int count = 0;
	for (uint32_t i = 0; i < entities.size(); i++)
	{
		for (uint32_t j = 0; j < entities[i].entityCount; j += moveBatchSize)
		{
			datas[count].pos = &reinterpret_cast<ECS::Pos*>(entities[i].ppComps[0])[j];
			datas[count].vel = &reinterpret_cast<Test::Velocity*>(entities[i].ppComps[1])[j];
			datas[count].batchSize = std::min(moveBatchSize, entities[i].entityCount - j);
			//JobFunc(&datas[count]);
			count++;
		}
	}
	assert(moveCounter.counter == 0);
	moveCounter.counter = count;
	pJobSystem->KickJobsWithPriority(jobDescs, count);
}

void R::Test::MoveSystem::WaitForCompletion()
{
	pJobSystem->WaitForCounter(&moveCounter);
}

void R::Test::MoveSystem::JobFunc(void* param, uint32_t tid)
{
	JobData* data = reinterpret_cast<JobData*>(param);
	for (uint32_t k = 0; k < data->batchSize; k++)
	{
		data->pos[k].x += data->vel[k].x * 0.16f;
		if (data->pos[k].x > 100)
			data->pos[k].x -= 200.f;
		if (data->pos[k].x < -100)
			data->pos[k].x += 200.f;
		data->pos[k].y += data->vel[k].y * 0.16f;
		if (data->pos[k].y > 100)
			data->pos[k].y -= 200.f;
		if (data->pos[k].y < -100)
			data->pos[k].y += 200.f;
		data->pos[k].z += data->vel[k].z * 0.16f;
		if (data->pos[k].z > 100)
			data->pos[k].z -= 200.f;
		if (data->pos[k].z < -100)
			data->pos[k].z += 200.f;
	}
}

