#pragma once
#include "pch.h"
#include "GlobalRenderContext.h"
#include "ThreadRenderContext.h"
#include "JobSystem/JobSystem.h"
namespace R
{
	namespace Rendering
	{
		class RenderSystem
		{
		public:
			RenderSystem(const uint32_t width, const uint32_t height, const HWND windowHandle, Job::JobSystem& jobSystem);
			~RenderSystem();
			
		private:
			GlobalRenderContext					m_globalRenderContext;
			std::vector<ThreadRenderContext>	m_threadRenderContexts;
			struct JobData
			{

			};
			Job::JobSystem* const				m_pJobSystem;
			Job::JobSystem::JobCounter			m_renderCounter;
			JobData* datas = new JobData[1000]; // Store it on heap because stack ~ 4mb
			Job::JobSystem::JobDesc* jobDescs = new Job::JobSystem::JobDesc[1000]; // Store it on heap because stack ~ 4mb
			//static void JobFunc(void* param);
		};
	}
}