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
			explicit RenderContext(const std::uint32_t width, const std::uint32_t height, const HWND windowHandle, const std::uint32_t threadPoolSize);
			~RenderContext();

			DEL_DEFAULT_COPY_MOVE_CTORS(RenderContext)
	
			inline ID3D12Device*					GetDevice()								const	{ return m_device.Get(); }
			inline ID3D12CommandQueue*				GetCommandQueue()						const	{ return m_commandQueue.Get(); }
			inline ID3D12Resource*					GetCurrentRenderTarget()				const	{ return m_renderTargets[m_frameIndex].Get(); }
			inline DXGI_FORMAT						GetRenderTargetFormat()					const	{ return DXGI_FORMAT_R8G8B8A8_UNORM; }
			inline CD3DX12_CPU_DESCRIPTOR_HANDLE	GetCurrentRTVHandle()					const	{ return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);}
			inline ID3D12Resource*					GetCurrentDepthBuffer()					const	{ return m_depthStencil[m_frameIndex].Get(); }
			inline DXGI_FORMAT						GetDepthBufferFormat()					const	{ return DXGI_FORMAT_D32_FLOAT; }
			inline CD3DX12_CPU_DESCRIPTOR_HANDLE	GetCurrentDSVHandle()					const	{ return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_dsvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_dsvDescriptorSize); }
			inline ID3D12Fence*						GetFence()								const	{ return m_fence.Get(); }
			inline const CD3DX12_VIEWPORT*			GetViewPort()							const	{ return &m_viewport; }
			inline const CD3DX12_RECT*				GetScissorRect()						const	{ return &m_scissorRect; }
			inline IDXGISwapChain3*					GetSwapChain()							const	{ return m_swapChain.Get(); }
			inline float							GetAspectRatio()						const	{ return m_aspectRatio; }
			inline std::uint64_t*					GetFrameNumber()								{ return &m_frameNumber; }
			inline std::uint32_t					GetFrameIndex()							const	{ return m_frameIndex; }
			inline ThreadContext* 					GetThreadContext(std::uint32_t index)	const	{ return &m_threadRenderContexts[index]; }
			inline ID3D12CommandList* const*		GetCommandLists()						const	{ return CommandListCast(m_threadCommandLists); }
			inline FrameResource* 					GetCurrentFrameResource()						{ return &m_frameResources[m_frameIndex]; }
			inline Camera*							GetCamera()										{ return &m_camera; }

			inline void								AdvanceFrame()									{ m_frameIndex = (m_frameIndex + 1) % FrameBuffersCount; }

			void									WaitForGPU();
		private:
			void GetHardwareAdapter(IDXGIFactory1* pFactory, IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter = false);
			// API Objects
			ComPtr<ID3D12Device>						m_device;
			ComPtr<ID3D12CommandQueue>					m_commandQueue;
			std::uint32_t								m_width;
			std::uint32_t								m_height;
			float										m_aspectRatio;
			CD3DX12_VIEWPORT							m_viewport;
			CD3DX12_RECT								m_scissorRect;
			ComPtr<IDXGISwapChain3>						m_swapChain;

			ComPtr<ID3D12Resource>						m_renderTargets[FrameBuffersCount];
			ComPtr<ID3D12DescriptorHeap>				m_rtvHeap;
			std::uint32_t								m_rtvDescriptorSize;

			ComPtr<ID3D12Resource>						m_depthStencil[FrameBuffersCount];
			ComPtr<ID3D12DescriptorHeap>				m_dsvHeap;
			std::uint32_t								m_dsvDescriptorSize;

			// Synchronization objects.
			ComPtr<ID3D12Fence>							m_fence;
			std::uint64_t								m_frameNumber;
			HANDLE										m_eventHandle;

			// Thread Contexts
			std::uint32_t								m_numWorkerThreads;
			ThreadContext*								m_threadRenderContexts; 	
			ID3D12GraphicsCommandList**					m_threadCommandLists;

			// Frame Resources
			std::uint32_t								m_frameIndex;
			FrameResource								m_frameResources[FrameBuffersCount];

			// Camera
			Camera										m_camera;
		};
	}
}