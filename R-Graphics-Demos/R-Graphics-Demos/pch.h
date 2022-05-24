#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif

#ifndef NOMINMAX
#define NOMINMAX						// Exclude min max, use std::min, std::max.
#endif

#include <windows.h>

// DX12
#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include "d3dx12.h"
#ifdef _DEBUG
#include <dxgidebug.h>
#endif // DEBUG


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
