#pragma once
#include "pch.h"
#include "GraphicsUtils.h"

using Microsoft::WRL::ComPtr;
namespace R
{
	namespace Rendering
	{
		template<size_t NumBuffers>
		class ThreadRenderContext
		{
		public:
			ThreadRenderContext(ID3D12Device* device, const std::wstring& name, ID3D12PipelineState* defaultPSO = nullptr);
			~ThreadRenderContext();

			ThreadRenderContext()											= delete;
			ThreadRenderContext(ThreadRenderContext&)						= delete;
			ThreadRenderContext& operator = (const ThreadRenderContext&)	= delete;
			ThreadRenderContext(ThreadRenderContext&&)						= delete;
			ThreadRenderContext& operator = (ThreadRenderContext&&)			= delete;

			inline ID3D12GraphicsCommandList* GetCommandList() { return m_commandList.Get(); }
			void Reset(uint32_t index, ID3D12PipelineState* initialState = nullptr);

		private:
			ComPtr<ID3D12CommandAllocator>		m_commandAllocator[NumBuffers];
			ComPtr<ID3D12GraphicsCommandList>	m_commandList;
			std::wstring						m_name;
		};
	}
}

template<size_t NumBuffers>
R::Rendering::ThreadRenderContext<NumBuffers>::ThreadRenderContext(ID3D12Device* device, const std::wstring& name, ID3D12PipelineState* defaultPSO)
	:m_name(name)
{
	static_assert(NumBuffers > 0);
	std::wstring n;
	for (size_t i = 0; i < NumBuffers; i++)
	{
		LogErrorIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator[i])));
		n = name + L"_" + std::to_wstring(i);
		LogErrorIfFailed(m_commandAllocator[i]->SetName(n.c_str()));
	}
	LogErrorIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator[0].Get(), defaultPSO, IID_PPV_ARGS(&m_commandList)));
	//m_commandList->Close();
}

template<size_t NumBuffers>
R::Rendering::ThreadRenderContext<NumBuffers>::~ThreadRenderContext()
{
	R_LOG_DEBUG(L"Destroying Allocator {}", m_name.c_str());
	//for (size_t i = 0; i < NumBuffers; i++)
	//{
	//	m_commandAllocator[i].Reset();
	//}
	//m_commandList.Reset();
}

template<size_t NumBuffers>
void R::Rendering::ThreadRenderContext<NumBuffers>::Reset(uint32_t index, ID3D12PipelineState* initialState)
{
	LogErrorIfFailed(m_commandAllocator[index]->Reset());
	LogErrorIfFailed(m_commandList->Reset(m_commandAllocator[index].Get(), initialState));
}
