#pragma once
#include "pch.h"
#include "DX12Helper.h"

using Microsoft::WRL::ComPtr;
namespace R
{
	namespace Rendering
	{
		class GlobalRenderContext
		{
		public:
			GlobalRenderContext() = delete;
			GlobalRenderContext(GlobalRenderContext&) = delete;
			GlobalRenderContext(GlobalRenderContext&&) = delete;
			GlobalRenderContext(const uint32_t width, const uint32_t height, const HWND windowHandle);
			~GlobalRenderContext();
	
			inline ID3D12Device* GetDevice() { return m_device.Get(); }
			inline ID3D12CommandQueue* GetCommandQueue() { return m_commandQueue.Get(); }
		private:
			void GetHardwareAdapter(IDXGIFactory1* pFactory, IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter = false);
			ComPtr<ID3D12Device>						m_device;
			ComPtr<ID3D12CommandQueue>					m_commandQueue;

			uint32_t									m_width;
			uint32_t									m_height;
			float										m_aspectRatio;
			CD3DX12_VIEWPORT							m_viewport;
			CD3DX12_RECT								m_scissorRect;
			ComPtr<IDXGISwapChain3>						m_swapChain;

			ComPtr<ID3D12Resource>						m_renderTargets[FrameCount];
			ComPtr<ID3D12DescriptorHeap>				m_rtvHeap;
			uint32_t									m_rtvDescriptorSize;
			
			// Synchronization objects.
			uint32_t									m_frameIndex;
			HANDLE										m_fenceEvent;
			ComPtr<ID3D12Fence>							m_fence;
			uint64_t									m_fenceValue;
		};
	}
}