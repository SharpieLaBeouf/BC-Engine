#pragma once

#include "Platform.h"

#include <filesystem>

#include <GLFW/glfw3.h>

#if defined(BC_PLATFORM_WINDOWS)

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#elif defined(BC_PLATFORM_LINUX)

#include "tinyfiledialogs.h"

#endif

namespace BC::Util
{
    
    static bool PathHasSubDirectory(const std::filesystem::path& directory_path) 
    {
        for (const auto& entry : std::filesystem::directory_iterator(directory_path))
            if (entry.path().parent_path() == directory_path && entry.is_directory())
                return true;
        return false;
    }

    static bool IsPathHidden(const std::filesystem::path& p)
    {
        #if defined(BC_PLATFORM_WINDOWS)

            DWORD attributes = GetFileAttributesW(p.c_str());
            if (attributes != INVALID_FILE_ATTRIBUTES) 
            {
                return (attributes & FILE_ATTRIBUTE_HIDDEN) != 0;
            }
            return false;

        #elif defined(BC_PLATFORM_MAC) || defined(BC_PLATFORM_LINUX)

            // Unix-like system check - file name starts with a dot
            return !p.filename().empty() && p.filename().string().front() == '.';

        #else

            return false;

        #endif
    }
}