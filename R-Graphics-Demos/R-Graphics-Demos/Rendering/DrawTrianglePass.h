#pragma once
#include "pch.h"
#include "JobSystem/JobSystem.h"
#include "GlobalRenderContext.h"
#include "ThreadRenderContext.h"
#include "Renderables.h"
namespace R
{
	namespace Rendering
	{
		class DrawTrianglePass
		{
			DrawTrianglePass(GlobalRenderContext* globalContext, ThreadRenderContext* threadContextArr, Job::JobSystem* jobSystem, Job::JobSystem::JobCounter* counter);
			~DrawTrianglePass();
		private:
			GlobalRenderContext*		m_pGlobalRenderContext;
			ThreadRenderContext*		m_pThreadRenderContextArray;
			Job::JobSystem*				m_pJobSystem;
			Job::JobSystem::JobCounter* m_pJobCounter;
			ComPtr<ID3D12RootSignature> m_rootSignature;
			ComPtr<ID3D12PipelineState> m_pipelineState;
		};
	}
}