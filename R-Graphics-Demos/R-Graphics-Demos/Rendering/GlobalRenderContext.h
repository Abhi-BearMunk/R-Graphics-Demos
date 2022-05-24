#pragma once
#include "pch.h"
#include "GraphicsUtils.h"

using Microsoft::WRL::ComPtr;
namespace R
{
	namespace Rendering
	{
		class GlobalRenderContext
		{
		public:
			GlobalRenderContext(const uint32_t width, const uint32_t height, const HWND windowHandle);
			~GlobalRenderContext();

			GlobalRenderContext()											= delete;
			GlobalRenderContext(GlobalRenderContext&)						= delete;
			GlobalRenderContext& operator = (const GlobalRenderContext&)	= delete;
			GlobalRenderContext(GlobalRenderContext&&)						= delete;
			GlobalRenderContext& operator = (GlobalRenderContext&&)			= delete;
	
			inline ID3D12Device* GetDevice() { return m_device.Get(); }
			inline ID3D12CommandQueue* GetCommandQueue() { return m_commandQueue.Get(); }
			inline ID3D12Resource* GetRenderTarget(uint32_t index) { return m_renderTargets[index].Get(); }
			inline ID3D12DescriptorHeap* GetRTVHeap() { return m_rtvHeap.Get(); }
			inline uint32_t GetRTVDescriptoSize() { return m_rtvDescriptorSize; }
			inline ID3D12Fence* GetFence() { return m_fence.Get(); }
			inline const CD3DX12_VIEWPORT* GetViewPort() { return &m_viewport; }
			inline const CD3DX12_RECT* GetScissorRect() { return &m_scissorRect; }
			inline IDXGISwapChain3* GetSwapChain() { return m_swapChain.Get(); }
			inline float GetAspectRatio() { return m_aspectRatio; }
			inline uint64_t* GetFrameNumber() { return &m_frameNumber; }

			void WaitForGPU();
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

			ComPtr<ID3D12Resource>						m_renderTargets[FrameBuffersCount];
			ComPtr<ID3D12DescriptorHeap>				m_rtvHeap;
			uint32_t									m_rtvDescriptorSize;

			// Synchronization objects.
			uint32_t									m_frameIndex;
			ComPtr<ID3D12Fence>							m_fence;
			uint64_t									m_frameNumber;
			HANDLE										m_eventHandle;
		};
	}
}