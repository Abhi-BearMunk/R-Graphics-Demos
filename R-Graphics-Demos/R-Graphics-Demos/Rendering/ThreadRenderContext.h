#pragma once
#include "pch.h"
#include "DX12Helper.h"
#include "GlobalRenderContext.h"

using Microsoft::WRL::ComPtr;
namespace R
{
	namespace Rendering
	{
		class ThreadRenderContext
		{
		public:
			ThreadRenderContext(ID3D12Device* device, ID3D12PipelineState* defaultPSO = nullptr);
			~ThreadRenderContext();

			ThreadRenderContext()											= delete;
			ThreadRenderContext(ThreadRenderContext&)						= delete;
			ThreadRenderContext& operator = (const ThreadRenderContext&)	= delete;
			ThreadRenderContext(ThreadRenderContext&&)						= delete;
			ThreadRenderContext& operator = (ThreadRenderContext&&)			= delete;

			inline ID3D12CommandList* GetCommandList() { return m_commandList.Get(); }
			void Reset(uint32_t index, ID3D12PipelineState* initialState = nullptr);
		private:
			ComPtr<ID3D12CommandAllocator>		m_commandAllocator[FrameBuffersCount];
			ComPtr<ID3D12GraphicsCommandList>	m_commandList;
		};
	}
}