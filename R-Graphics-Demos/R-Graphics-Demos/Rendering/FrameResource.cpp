#include "pch.h"
#include "FrameResource.h"

R::Rendering::FrameResource::FrameResource()
	:m_count(0), m_state(FrameResourceState::e_free), m_fenceValue(0)
{
	m_eventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_eventHandle == nullptr)
	{
		LogErrorIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}
}

R::Rendering::FrameResource::~FrameResource()
{
	CloseHandle(m_eventHandle);
}

void R::Rendering::FrameResource::Submit(ID3D12CommandQueue* cmdQ, ID3D12Fence* fence, std::uint64_t* frameNumber)
{
	m_fenceValue = ++(*frameNumber);
	LogErrorIfFailed(cmdQ->Signal(fence, m_fenceValue));
	m_state = FrameResourceState::e_inUse;
}

void R::Rendering::FrameResource::Release(ID3D12Fence* fence)
{
	// If the next frame is not ready to be rendered yet, wait until it is ready.
	if (m_fenceValue != 0 && fence->GetCompletedValue() < m_fenceValue)
	{
		LogErrorIfFailed(fence->SetEventOnCompletion(m_fenceValue, m_eventHandle));
		WaitForSingleObjectEx(m_eventHandle, INFINITE, FALSE);
	}
	m_state = FrameResourceState::e_free;
}
