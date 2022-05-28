#pragma once
#include "pch.h"
#include "Utils/Logger.h"

namespace R
{
    namespace Rendering
    {
        constexpr std::uint32_t FrameBuffersCount = 3;

        inline std::string HrToString(HRESULT hr)
        {
            char s_str[64] = {};
            sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
            return std::string(s_str);
        }

        inline void LogErrorIfFailed(HRESULT hr, const std::string& reason = "")
        {
            if (FAILED(hr))
            {
                R_LOG_ERROR("HR error: {} - {}", HrToString(hr), reason);
            }
        }

        // Assign a name to the object to aid with debugging.
#if defined(_DEBUG) || defined(DBG)
        inline void SetName(ID3D12Object* pObject, LPCWSTR name)
        {
            pObject->SetName(name);
        }
        inline void SetNameIndexed(ID3D12Object* pObject, LPCWSTR name, UINT index)
        {
            WCHAR fullName[50];
            if (swprintf_s(fullName, L"%s[%u]", name, index) > 0)
            {
                pObject->SetName(fullName);
            }
        }
#else
        inline void SetName(ID3D12Object*, LPCWSTR)
        {
        }
        inline void SetNameIndexed(ID3D12Object*, LPCWSTR, UINT)
        {
        }
#endif

#define NAME_D3D12_OBJECT(x) SetName(x, L#x)
#define NAME_D3D12_OBJECT_INDEXED(x, n) SetNameIndexed(x, L#x, n)

#define NAME_D3D12_COMPTR(x) SetName((x).Get(), L#x)
#define NAME_D3D12_COMPTR_INDEXED(x, n) SetNameIndexed((x)[n].Get(), L#x, n)

        void CommandCreateBufferFromData(ID3D12Device* device,
            ID3D12GraphicsCommandList* cmdList,
            ID3D12Resource** dst,
            ID3D12Resource** uploadBuffer,
            void* data, size_t sizeInBytes,
            D3D12_RESOURCE_STATES stateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
    }
}