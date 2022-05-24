#pragma once
#include "pch.h"
#include "Utils/Logger.h"

namespace R
{
    namespace Rendering
    {
        constexpr uint32_t FrameBuffersCount = 2;

        inline std::string HrToString(HRESULT hr)
        {
            char s_str[64] = {};
            sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
            return std::string(s_str);
        }

        inline void LogErrorIfFailed(HRESULT hr)
        {
            if (FAILED(hr))
            {
                R_LOG_ERROR(HrToString(hr));
            }
        }

        void CommandCreateBufferFromData(ID3D12Device* device,
            ID3D12GraphicsCommandList* cmdList,
            ID3D12Resource** dst,
            ID3D12Resource** uploadBuffer,
            void* data, size_t sizeInBytes,
            D3D12_RESOURCE_STATES stateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    }
}