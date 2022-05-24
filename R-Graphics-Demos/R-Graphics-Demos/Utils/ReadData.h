#pragma once
#include "pch.h"
#include "Logger.h"

// TODO: Copy Pasta : Cleanup later
namespace R
{
    namespace Utils
    {
        inline void GetAbsolutePath(_In_z_ const wchar_t* relativePath, _Outptr_ wchar_t* fullPath)
        {
#if !defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)

            wchar_t drive[_MAX_DRIVE];
            wchar_t path[_MAX_PATH];

            wchar_t moduleName[_MAX_PATH];
            if (!GetModuleFileNameW(nullptr, moduleName, _MAX_PATH))
                R_LOG_ERROR(L"GetModuleFileName {}", moduleName);

            if (_wsplitpath_s(moduleName, drive, _MAX_DRIVE, path, _MAX_PATH, nullptr, 0, nullptr, 0))
                R_LOG_ERROR(L"_wsplitpath_s {}", moduleName);

            if (_wmakepath_s(fullPath, _MAX_PATH, drive, path, relativePath, nullptr))
                R_LOG_ERROR(L"_wmakepath_s {} / {}", moduleName);
#else
            * fullPath = *relativePath;
#endif
        }

        inline std::vector<uint8_t> ReadData(_In_z_ const wchar_t* name)
        {
            std::ifstream inFile(name, std::ios::in | std::ios::binary | std::ios::ate);

#if !defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
            if (!inFile)
            {
                wchar_t filename[_MAX_PATH];
                GetAbsolutePath(name, filename);

                inFile.open(filename, std::ios::in | std::ios::binary | std::ios::ate);
            }
#endif

            if (!inFile)
                R_LOG_ERROR(L"ReadData {}", name);

            std::streampos len = inFile.tellg();
            if (!inFile)
                R_LOG_ERROR(L"ReadData {}", name);

            std::vector<uint8_t> blob;
            blob.resize(size_t(len));

            inFile.seekg(0, std::ios::beg);
            if (!inFile)
                R_LOG_ERROR(L"ReadData {}", name);

            inFile.read(reinterpret_cast<char*>(blob.data()), len);
            if (!inFile)
                R_LOG_ERROR(L"ReadData {}", name);

            inFile.close();

            return blob;
        }
    }
}
