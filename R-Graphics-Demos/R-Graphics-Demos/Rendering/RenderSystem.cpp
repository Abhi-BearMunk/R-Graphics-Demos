#include "pch.h"
#include "RenderSystem.h"

R::Rendering::RenderSystem::RenderSystem(const uint32_t width, const uint32_t height, const HWND windowHandle, Job::JobSystem& jobSystem)
	:m_globalRenderContext(width, height, windowHandle), m_pJobSystem(&jobSystem), m_threadRenderContexts(jobSystem.GetNumWorkers(), m_globalRenderContext.GetDevice())
{
}

R::Rendering::RenderSystem::~RenderSystem()
{
}
