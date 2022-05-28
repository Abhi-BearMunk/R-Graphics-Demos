#pragma once
#include "pch.h"
#include "JobSystem/JobSystem.h"
#include "Utils/ReadData.h"
#include "RenderContext.h"
#include "ThreadRenderContext.h"
#include "Renderables.h"
#include "FrameResource.h"
#include "PrimitivesGenerator.h"
namespace R
{
	namespace Rendering
	{
		class BasePass
		{
			friend class BasePass;
		public:
			BasePass(RenderContext* renderContext, Job::JobSystem* jobSystem);
			~BasePass();
			void Init(ID3D12GraphicsCommandList* cmdList);
			void Update();
			void WaitForCompletion();
		private:
			void SetupRSAndPSO();
			void SetupVertexBuffer(ID3D12GraphicsCommandList* cmdList);
			struct JobConstData
			{
				inline JobConstData(RenderContext* _renderContext) : renderContext(_renderContext){}
				RenderContext* renderContext;
			};

			struct JobData
			{
				JobConstData* constData;
				std::uint32_t startIndex;
				std::uint32_t batchSize;
			};

			JobConstData								m_jobConstData;

			Job::JobSystem*								m_pJobSystem;
			Job::JobSystem::JobCounter					m_jobCounter;
			JobData*									m_jobDatas = new JobData[ECS::MAX_ENTITIES_PER_ARCHETYPE];
			Job::JobSystem::JobDesc*					m_jobDescs = new Job::JobSystem::JobDesc[ECS::MAX_ENTITIES_PER_ARCHETYPE];
			static void JobFunc(void* param, std::uint32_t tid);

			ComPtr<ID3D12RootSignature>					m_rootSignature;
			ComPtr<ID3D12PipelineState>					m_pipelineState;
			Microsoft::WRL::ComPtr<ID3D12Resource>		m_vertexBuffer;
			Microsoft::WRL::ComPtr<ID3D12Resource>      m_vertexBufferUploader;
			Microsoft::WRL::ComPtr<ID3D12Resource>      m_indexBuffer;
			Microsoft::WRL::ComPtr<ID3D12Resource>      m_indexBufferUploader;
			D3D12_VERTEX_BUFFER_VIEW                    m_vertexBufferView;
			D3D12_INDEX_BUFFER_VIEW                     m_indexBufferView;
		};
	}
}