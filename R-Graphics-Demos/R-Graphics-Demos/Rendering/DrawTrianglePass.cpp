#include "pch.h"
#include "DrawTrianglePass.h"

R::Rendering::DrawTrianglePass::DrawTrianglePass(GlobalRenderContext* globalContext, ThreadRenderContext* threadContextArr, Job::JobSystem* jobSystem, Job::JobSystem::JobCounter* counter)
	:m_pGlobalRenderContext(globalContext), m_pThreadRenderContextArray(threadContextArr), m_pJobSystem(jobSystem), m_pJobCounter(counter)
{
}

R::Rendering::DrawTrianglePass::~DrawTrianglePass()
{
}
