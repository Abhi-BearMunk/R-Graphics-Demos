#pragma once
#include "pch.h"
#include "ECS/PredefinedComponents.h"
#include "Test/TestComponents.h"
#include "ECS/World.h"
#include "JobSystem/JobSystem.h"
namespace R
{
	namespace Test
	{
		class MoveSystem
		{
		public:
			MoveSystem(ECS::World& world, Job::JobSystem& jobSystem);
			~MoveSystem();
			void Update(const float& dt);
			void WaitForCompletion();
		private:
			std::vector<ECS::Interest<2>> entities;
			struct JobData
			{
				ECS::Pos* pos;
				Test::Velocity* vel;
				std::uint32_t batchSize;
			};
			Job::JobSystem* const pJobSystem;
			Job::JobSystem::JobCounter moveCounter;
			std::uint32_t moveBatchSize = 10000;
			JobData* datas = new JobData[ECS::MAX_ENTITIES_PER_ARCHETYPE]; // Store it on heap because stack ~ 4mb
			Job::JobSystem::JobDesc* jobDescs = new Job::JobSystem::JobDesc[ECS::MAX_ENTITIES_PER_ARCHETYPE]; // Store it on heap because stack ~ 4mb
			static void JobFunc(void* param, std::uint32_t tid);
		};
	}
}

