#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

#ifndef NOMINMAX
#define NOMINMAX						// Exclude min max, use std::min, std::max.
#endif

#include <windows.h>

// DX12 Core
#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include "d3dx12.h"
#ifdef _DEBUG
#include <dxgidebug.h>
#endif // DEBUG

// DX12 Helpers
#include <DirectXTex/DirectXTex/DirectXTex.h>

using Microsoft::WRL::ComPtr;

// SpdLog
#ifndef SPDLOG_WCHAR_TO_UTF8_SUPPORT
#define SPDLOG_WCHAR_TO_UTF8_SUPPORT
#endif
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

// STD
#include <cstdint>
#include <fstream>
#include <cassert>
#include <type_traits>
#include <algorithm>
#include <string>
#include <format>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

// STL
#include <vector>
#include <queue>
#include <unordered_map>

#define DEL_DEFAULT_CTOR(X) X() = delete;
#define DEL_COPY_CTOR(X) X(X&) = delete; \
X& operator = (const X&) = delete; 
#define DEL_MOVE_CTOR(X) X(X&&) = delete; \
X& operator = (X&&) = delete; 

#define DEL_DEFAULT_COPY_MOVE_CTORS(X) DEL_DEFAULT_CTOR(X) \
DEL_COPY_CTOR(X) \
DEL_MOVE_CTOR(X)

#define SIZE_OF_16(x) sizeof(x) / sizeof(std::uint16_t)
#define SIZE_OF_32(x) sizeof(x) / sizeof(std::uint32_t)
#define SIZE_OF_64(x) sizeof(x) / sizeof(std::uint64_t)

#define _GLUE(a, b) a##b
#define GLUE(a, b) _GLUE(a, b)
#define _NAME_WCHAR(x) L#x
#define NAME_WCHAR(x) _NAME_WCHAR(x)

namespace R
{
	namespace Constants
	{
		constexpr float DegToRad = DirectX::XM_PI / 180.0f;
		constexpr float RadToDeg = 180.0f / DirectX::XM_PI;
	}
}
