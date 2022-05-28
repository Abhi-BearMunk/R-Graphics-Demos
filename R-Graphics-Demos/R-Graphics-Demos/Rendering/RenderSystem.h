#pragma once
#include "pch.h"
#include "RenderContext.h"
#include "ThreadRenderContext.h"
#include "JobSystem/JobSystem.h"
#include "ECS/World.h"
#include "ECS/PredefinedComponents.h"
#include "FrameResource.h"
#include "ResourceDatas.h"
#include "BasePass.h"

namespace R
{
	namespace Rendering
	{
		class RenderSystem
		{
		public:
			RenderSystem(const std::uint32_t width, const std::uint32_t height, const HWND windowHandle, Job::JobSystem& jobSystem, const ResourceData* externalResources = nullptr);
			~RenderSystem();
			void Render();
			inline RenderContext* GetRenderContext() { return &m_renderContext; }
		private:

			void CommandUploadResourcesToGPU(const ResourceData* resourceData);
			static void TextureUploadJobFunc(void* param, std::uint32_t tid);
			void CloseAndExecuteThreadCommandLists();

			// Resource upload jobs
			struct ResourceUploadJobConstData
			{
				RenderContext* renderContext;
				const ResourceData* resourceData;
				ComPtr<ID3D12Resource>* textureUploaders;
			};

			struct ResourceUploadJobData
			{
				ResourceUploadJobConstData* constData;
				std::uint32_t startIndex;
				std::uint32_t batchSize;
			};

			RenderContext								m_renderContext;
			Job::JobSystem* const						m_pJobSystem;

			// Passes and commandlists
			ThreadRenderContext<FrameBuffersCount>		m_beginFrame;
			ThreadRenderContext<FrameBuffersCount>		m_endFrame;
			BasePass									m_basePass;
		};
	}
}