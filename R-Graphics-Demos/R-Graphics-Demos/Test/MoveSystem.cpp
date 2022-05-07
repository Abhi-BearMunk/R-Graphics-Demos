#include "pch.h"
#include "MoveSystem.h"
#include <stdlib.h> 

R::Test::MoveSystem::MoveSystem(ECS::World& world, Job::JobSystem& jobSystem)
	:pJobSystem(&jobSystem)
{
	world.InterestedIn<ECS::Pos, Test::Velocity>(entities);
	for (int i = 0; i < ECS::MAX_ENTITIES_PER_COMPONENT_PER_ARCHETYPE; i++)
	{
		jobDescs[i].jobFunc = &MoveSystem::JobFunc;
		jobDescs[i].param = &datas[i];
		jobDescs[i].pCounter = &moveCounter;
	}
}

void R::Test::MoveSystem::Update(const float& dt)
{
	int count = 0;
	for (uint32_t i = 0; i < entities.size(); i++)
	{
		for (uint32_t j = 0; j < entities[i].entityCount; j++)
		{
			datas[count].pos = &reinterpret_cast<ECS::Pos*>(entities[i].ppComps[0])[i];
			datas[count].vel = &reinterpret_cast<Test::Velocity*>(entities[i].ppComps[1])[i];
			auto data = &datas[count];
			for (int k = 0; k < 5000; k++)
			{
				data->pos->x += data->pos->x > 0 ? data->vel->x * data->vel->x : -2 * data->pos->y;
			}
			count++;
		}
	}
	moveCounter.counter += count;
	pJobSystem->KickJobsWithPriority(jobDescs, count);
}

void R::Test::MoveSystem::WaitForCompletion()
{
	pJobSystem->WaitForCounter(&moveCounter);
}

void R::Test::MoveSystem::JobFunc(void* param)
{
	JobData* data = reinterpret_cast<JobData*>(param);
	for (int k = 0; k < 5000; k++)
	{
		data->pos->x += data->pos->x > 0 ? data->vel->x * data->vel->x : -2 * data->pos->y;
	}
}

