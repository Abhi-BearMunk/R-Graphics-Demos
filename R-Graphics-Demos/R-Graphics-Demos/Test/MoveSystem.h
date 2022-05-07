#pragma once
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
			void Update(const float& dt);
			void WaitForCompletion();
		private:
			std::vector<ECS::Interest<2>> entities;
			struct JobData
			{
				ECS::Pos* pos;
				Test::Velocity* vel;
			};
			Job::JobSystem* const pJobSystem;
			Job::JobSystem::JobCounter moveCounter;
			JobData* datas = new JobData[ECS::MAX_ENTITIES_PER_COMPONENT_PER_ARCHETYPE]; // Store it on heap because stack ~ 4mb
			Job::JobSystem::JobDesc* jobDescs = new Job::JobSystem::JobDesc[ECS::MAX_ENTITIES_PER_COMPONENT_PER_ARCHETYPE]; // Store it on heap because stack ~ 4mb
			static void JobFunc(void* param);
		};
	}
}

