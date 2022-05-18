#pragma once
#include "pch.h"
#include "Utils/Logger.h"

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