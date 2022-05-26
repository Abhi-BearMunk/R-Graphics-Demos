#pragma once
#include "pch.h"
#include "RenderContext.h"
#include "ThreadRenderContext.h"
#include "JobSystem/JobSystem.h"
#include "ECS/World.h"
#include "ECS/PredefinedComponents.h"
#include "FrameResource.h"
#include "BasePass.h"

namespace R
{
	namespace Rendering
	{
		class RenderSystem
		{
		public:
			RenderSystem(const std::uint32_t width, const std::uint32_t height, const HWND windowHandle, ECS::World& world, Job::JobSystem& jobSystem);
			~RenderSystem();
			void Render();
			inline RenderContext* GetRenderContext() { return &m_renderContext; }
		private:
			RenderContext								m_renderContext;
			Job::JobSystem* const						m_pJobSystem;

			// Passes and commandlists
			ThreadRenderContext<FrameBuffersCount>		m_beginFrame;
			ThreadRenderContext<FrameBuffersCount>		m_endFrame;
			BasePass									m_basePass;
		};
	}
}