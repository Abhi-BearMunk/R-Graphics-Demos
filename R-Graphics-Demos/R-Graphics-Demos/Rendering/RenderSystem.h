#pragma once
#include "pch.h"
#include "GlobalRenderContext.h"
#include "ThreadRenderContext.h"
#include "JobSystem/JobSystem.h"
#include "ECS/World.h"
#include "ECS/PredefinedComponents.h"
#include "FrameResource.h"

namespace R
{
	namespace Rendering
	{
		class RenderSystem
		{
		public:
			RenderSystem(const uint32_t width, const uint32_t height, const HWND windowHandle, ECS::World& world, Job::JobSystem& jobSystem);
			~RenderSystem();
			void Update(const float& dt);
			void WaitForUpdate();
			void Render();
			void WaitForRender();
		private:
			static void UpdateJobFunc(void* param, uint32_t tid);
			static void RenderJobFunc(void* param, uint32_t tid);

			struct UpdateJobData
			{
				ECS::Pos* pos;
				FrameResource* frameResource;
				uint32_t startIndex;
				uint32_t batchSize;
			};

			struct RenderJobData
			{
				GlobalRenderContext* globalRenderContext;
				ThreadRenderContext* threadRenderContextArr;
				FrameResource* frameResource;
				uint32_t startIndex;
				uint32_t batchSize;
			};

			GlobalRenderContext					m_globalRenderContext;
			ThreadRenderContext*				m_threadRenderContexts; // TODO : This is on the heap because of the variable number of threads, can we do better?	
			FrameResource						m_frameResources[FrameBuffersCount];
			uint32_t							m_currentFrameResourceIndex;
			Job::JobSystem* const				m_pJobSystem;

			// Renderable updates
			std::vector<ECS::Interest<1>>		m_entities;
			UpdateJobData*						m_updateJobDatas = new UpdateJobData[ECS::MAX_ENTITIES_PER_ARCHETYPE];
			Job::JobSystem::JobCounter			m_updateCounter;
			Job::JobSystem::JobDesc*			m_updateJobDescs = new Job::JobSystem::JobDesc[ECS::MAX_ENTITIES_PER_ARCHETYPE]; // Store it on heap because stack ~ 4mb

			// Render commands
			Job::JobSystem::JobCounter			m_renderCounter;
		};
	}
}