#pragma once
#include "pch.h"
#include "GraphicsUtils.h"
#include "FrameResource.h"
#include "ThreadRenderContext.h"

using Microsoft::WRL::ComPtr;
namespace R
{
	namespace Rendering
	{
		using ThreadContext = ThreadRenderContext<FrameBuffersCount>;
		struct Camera
		{
			XMFLOAT3 position { 0.f, 0.f, 0.f};
			XMFLOAT4 orientation { 0.f, 0.f, 0.f, 1.f };
			XMFLOAT3 right{ 1.f, 0.f, 0.f };
			XMFLOAT3 up{ 0.f, 1.f, 0.f };
			XMFLOAT3 forward{ 0.f, 0.f, 1.f };
			float nearPlane = 0.01f;
			float farPlane = 1000.0f;
			float fov = 60.0f;
		};
		class RenderContext
		{
		public:
			explicit RenderContext(const uint32_t width, const uint32_t height, const HWND windowHandle, const uint32_t threadPoolSize);
			~RenderContext();

			DEL_DEFAULT_COPY_MOVE_CTORS(RenderContext)
	
			inline ID3D12Device*				GetDevice()							const	{ return m_device.Get(); }
			inline ID3D12CommandQueue*			GetCommandQueue()					const	{ return m_commandQueue.Get(); }
			inline ID3D12Resource*				GetRenderTarget(uint32_t index)		const	{ return m_renderTargets[index].Get(); }
			inline ID3D12DescriptorHeap*		GetRTVHeap()						const	{ return m_rtvHeap.Get(); }
			inline uint32_t						GetRTVDescriptorSize()				const	{ return m_rtvDescriptorSize; }
			inline ID3D12Fence*					GetFence()							const	{ return m_fence.Get(); }
			inline const CD3DX12_VIEWPORT*		GetViewPort()						const	{ return &m_viewport; }
			inline const CD3DX12_RECT*			GetScissorRect()					const	{ return &m_scissorRect; }
			inline IDXGISwapChain3*				GetSwapChain()						const	{ return m_swapChain.Get(); }
			inline float						GetAspectRatio()					const	{ return m_aspectRatio; }
			inline uint64_t*					GetFrameNumber()							{ return &m_frameNumber; }
			inline uint32_t						GetFrameIndex()						const	{ return m_frameIndex; }
			inline ThreadContext* 				GetThreadContext(uint32_t index)	const	{ return &m_threadRenderContexts[index]; }
			inline ID3D12CommandList* const*	GetCommandLists()					const	{ return CommandListCast(m_threadCommandLists); }
			inline FrameResource* 				GetCurrentFrameResource()					{ return &m_frameResources[m_frameIndex]; }
			inline Camera*						GetCamera()									{ return &m_camera; }

			inline void							AdvanceFrame()								{ m_frameIndex = (m_frameIndex + 1) % FrameBuffersCount; }

			void								WaitForGPU();
		private:
			void GetHardwareAdapter(IDXGIFactory1* pFactory, IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter = false);
			// API Objects
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
			ComPtr<ID3D12Fence>							m_fence;
			uint64_t									m_frameNumber;
			HANDLE										m_eventHandle;

			// Thread Contexts
			uint32_t									m_numWorkerThreads;
			ThreadContext*								m_threadRenderContexts; 	
			ID3D12GraphicsCommandList**					m_threadCommandLists;

			// Frame Resources
			uint32_t									m_frameIndex;
			FrameResource								m_frameResources[FrameBuffersCount];

			// Camera
			Camera										m_camera;
		};
	}
}