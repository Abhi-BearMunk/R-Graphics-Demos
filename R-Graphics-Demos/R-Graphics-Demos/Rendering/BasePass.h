#pragma once
#include "pch.h"
#include "JobSystem/JobSystem.h"
#include "Utils/ReadData.h"
#include "GlobalRenderContext.h"
#include "ThreadRenderContext.h"
#include "Renderables.h"
#include "FrameResource.h"
namespace R
{
	namespace Rendering
	{
		class BasePass
		{
		public:
			BasePass(GlobalRenderContext* globalContext, ThreadRenderContext<FrameBuffersCount>* threadContextArr, Job::JobSystem* jobSystem);
			~BasePass();
			void Init(ID3D12GraphicsCommandList* cmdList);
			void Update(FrameResource* frameResource, uint32_t frameIndex);
			void WaitForCompletion();
		private:
			void SetupRSAndPSO();
			void SetupVertexBuffer(ID3D12GraphicsCommandList* cmdList);
			struct JobData
			{
				GlobalRenderContext* globalRenderContext;
				ThreadRenderContext<FrameBuffersCount>* threadRenderContextArr;
				FrameResource* frameResource;
				uint32_t startIndex;
				uint32_t batchSize;
			};

			Job::JobSystem*								m_pJobSystem;
			Job::JobSystem::JobCounter					m_jobCounter;
			JobData*									m_jobDatas = new JobData[ECS::MAX_ENTITIES_PER_ARCHETYPE];
			Job::JobSystem::JobDesc*					m_jobDescs = new Job::JobSystem::JobDesc[ECS::MAX_ENTITIES_PER_ARCHETYPE];
			static void JobFunc(void* param, uint32_t tid);

			GlobalRenderContext*						m_pGlobalRenderContext;
			ThreadRenderContext<FrameBuffersCount>*		m_pThreadRenderContextArray;


			ComPtr<ID3D12RootSignature>					m_rootSignature;
			ComPtr<ID3D12PipelineState>					m_pipelineState;
			Microsoft::WRL::ComPtr<ID3D12Resource>		m_vertexBuffer;
			Microsoft::WRL::ComPtr<ID3D12Resource>      m_vertexUploadBuffer;
			Microsoft::WRL::ComPtr<ID3D12Resource>      m_indexBuffer;
			D3D12_VERTEX_BUFFER_VIEW                    m_vertexBufferView;
			D3D12_INDEX_BUFFER_VIEW                     m_indexBufferView;
		};
	}
}