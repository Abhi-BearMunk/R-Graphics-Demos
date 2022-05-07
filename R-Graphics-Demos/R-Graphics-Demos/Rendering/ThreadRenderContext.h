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
			ThreadRenderContext() = delete;
			//ThreadRenderContext(ThreadRenderContext&) = delete;
			//ThreadRenderContext(ThreadRenderContext&&) = delete;
			ThreadRenderContext(ID3D12Device* device, ID3D12PipelineState* defaultPSO = nullptr);
			~ThreadRenderContext();
		private:
			ComPtr<ID3D12CommandAllocator>		m_commandAllocator;
			ComPtr<ID3D12GraphicsCommandList>	m_commandList;
		};
	}
}