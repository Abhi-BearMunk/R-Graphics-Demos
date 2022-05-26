#include "pch.h"
#include "RenderContext.h"
#include "GraphicsUtils.h"
namespace R
{
	namespace Rendering
	{
		RenderContext::RenderContext(const std::uint32_t width, const std::uint32_t height, const HWND windowHandle, const std::uint32_t threadPoolSize)
            :m_width(width), 
            m_height(height), 
            m_frameIndex(FrameBuffersCount - 1), 
            m_frameNumber(0), 
            m_numWorkerThreads(threadPoolSize),
            m_threadRenderContexts(reinterpret_cast<ThreadContext*>(operator new[](sizeof(ThreadContext)* threadPoolSize))),
            m_threadCommandLists(new ID3D12GraphicsCommandList* [threadPoolSize]),
            m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
            m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height))
		{
            UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
            // Enable the debug layer (requires the Graphics Tools "optional feature").
            // NOTE: Enabling the debug layer after device creation will invalidate the active device.
            {
                ComPtr<ID3D12Debug> debugController;
                if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
                {
                    debugController->EnableDebugLayer();
                }
            }
            ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
            if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiInfoQueue.GetAddressOf()))))
            {
                dxgiFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;

                dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
                dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);
            }
#endif //_DEBUG

            ComPtr<IDXGIFactory4> factory;
            LogErrorIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

            ComPtr<IDXGIAdapter1> hardwareAdapter;
            GetHardwareAdapter(factory.Get(), &hardwareAdapter);

            LogErrorIfFailed(D3D12CreateDevice(
                hardwareAdapter.Get(),
                D3D_FEATURE_LEVEL_11_0,
                IID_PPV_ARGS(&m_device)
            ));
            // Describe and create the command queue.
            D3D12_COMMAND_QUEUE_DESC queueDesc = {};
            queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
            queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

            LogErrorIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));
            m_commandQueue->SetName(L"Graphics Command Queue");

            // Describe and create the swap chain.
            DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
            swapChainDesc.BufferCount = FrameBuffersCount;
            swapChainDesc.Width = m_width;
            swapChainDesc.Height = m_height;
            swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
            swapChainDesc.SampleDesc.Count = 1;

            ComPtr<IDXGISwapChain1> swapChain;
            LogErrorIfFailed(factory->CreateSwapChainForHwnd(
                m_commandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
                windowHandle,
                &swapChainDesc,
                nullptr,
                nullptr,
                &swapChain
            ));

            // This sample does not support fullscreen transitions.
            LogErrorIfFailed(factory->MakeWindowAssociation(windowHandle, DXGI_MWA_NO_ALT_ENTER));

            LogErrorIfFailed(swapChain.As(&m_swapChain));
            //m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

            // Create descriptor heaps.
            {
                // Describe and create a render target view (RTV) descriptor heap.
                D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
                rtvHeapDesc.NumDescriptors = FrameBuffersCount;
                rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
                rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
                LogErrorIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

                m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            }

            // Create frame resources.
            {
                CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

                // Create a RTV for each frame.
                for (std::uint32_t n = 0; n < FrameBuffersCount; n++)
                {
                    LogErrorIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
                    m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
                    rtvHandle.Offset(1, m_rtvDescriptorSize);
                }
            }

            // Set Window related stuff
            m_aspectRatio = static_cast<float>(m_width) / static_cast<float>(m_height);

            // Create fence
            LogErrorIfFailed(m_device->CreateFence(m_frameNumber, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
            m_frameNumber++;

            m_eventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
            if (m_eventHandle == nullptr)
            {
                LogErrorIfFailed(HRESULT_FROM_WIN32(GetLastError()));
            }

            // Create Thread Render constexts (Command lists and allocators)
            for (size_t i = 0; i < threadPoolSize; i++)
            {
                new (&m_threadRenderContexts[i]) ThreadContext(m_device.Get(), L"ThreadContext_" + std::to_wstring(i));
                LogErrorIfFailed(m_threadRenderContexts[i].GetCommandList()->Close());
                m_threadCommandLists[i] = m_threadRenderContexts[i].GetCommandList();
            }
		}

		RenderContext::~RenderContext()
		{
            WaitForGPU();
            CloseHandle(m_eventHandle);
            for (size_t i = 0; i < m_numWorkerThreads; i++)
            {
                m_threadRenderContexts[i].~ThreadRenderContext();
            }
            operator delete[](m_threadRenderContexts);

#ifdef _DEBUG
            ComPtr<IDXGIDebug1> dxgiDebug;
            if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
            {
                dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
            }
#endif //_DEBUG
		}

        void RenderContext::WaitForGPU()
        {
            m_frameNumber++;
            LogErrorIfFailed(m_commandQueue->Signal(m_fence.Get(), m_frameNumber));
            // Signal And Wait
            if (m_fence->GetCompletedValue() < m_frameNumber)
            {
                LogErrorIfFailed(m_fence->SetEventOnCompletion(m_frameNumber, m_eventHandle));
                WaitForSingleObjectEx(m_eventHandle, INFINITE, FALSE);
            }
        }

        void RenderContext::GetHardwareAdapter(IDXGIFactory1* pFactory, IDXGIAdapter1** ppAdapter, bool requestHighPerformanceAdapter)
        {
            *ppAdapter = nullptr;

            ComPtr<IDXGIAdapter1> adapter;

            ComPtr<IDXGIFactory6> factory6;
            if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
            {
                for (
                    UINT adapterIndex = 0;
                    DXGI_ERROR_NOT_FOUND != factory6->EnumAdapterByGpuPreference(
                        adapterIndex,
                        requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
                        IID_PPV_ARGS(&adapter));
                    ++adapterIndex)
                {
                    DXGI_ADAPTER_DESC1 desc;
                    adapter->GetDesc1(&desc);

                    if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                    {
                        // Don't select the Basic Render Driver adapter.
                        continue;
                    }

                    // Check to see whether the adapter supports Direct3D 12, but don't create the
                    // actual device yet.
                    if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
                    {
                        break;
                    }
                }
            }
            else
            {
                for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != pFactory->EnumAdapters1(adapterIndex, &adapter); ++adapterIndex)
                {
                    DXGI_ADAPTER_DESC1 desc;
                    adapter->GetDesc1(&desc);

                    if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                    {
                        // Don't select the Basic Render Driver adapter.
                        continue;
                    }

                    // Check to see whether the adapter supports Direct3D 12, but don't create the
                    // actual device yet.
                    if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
                    {
                        break;
                    }
                }
            }

            *ppAdapter = adapter.Detach();
        }
	}
}