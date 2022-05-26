#pragma once
#include "pch.h"
#include "ECS/World.h"
#include "ECS/PredefinedComponents.h"
#include "JobSystem/JobSystem.h"
#include "RenderContext.h"
#include "FrameResource.h"

namespace R
{
	namespace Rendering
	{
		class RenderableManager
		{
		public:
			RenderableManager(ECS::World& world, Job::JobSystem& jobSystem);
			~RenderableManager();
			void Update(RenderContext* renderContext);
			void WaitForUpdate();
		private:
			static void UpdateJobFunc(void* param, std::uint32_t tid);

			struct JobConstData
			{
				FrameResource* frameResource;
				XMFLOAT4X4 matVP;
			};
			struct UpdateJobData
			{
				ECS::Pos* pos;
				JobConstData* constData;
				std::uint32_t startIndex;
				std::uint32_t batchSize;
			};

			Job::JobSystem* const						m_pJobSystem;
			JobConstData								m_jobConstData;
			// Renderable updates
			std::vector<ECS::Interest<1>>				m_entities;
			UpdateJobData*								m_updateJobDatas = new UpdateJobData[ECS::MAX_ENTITIES_PER_ARCHETYPE];
			Job::JobSystem::JobCounter					m_updateCounter;
			Job::JobSystem::JobDesc*					m_updateJobDescs = new Job::JobSystem::JobDesc[ECS::MAX_ENTITIES_PER_ARCHETYPE]; // Store it on heap because stack ~ 4mb

		};
	}
}