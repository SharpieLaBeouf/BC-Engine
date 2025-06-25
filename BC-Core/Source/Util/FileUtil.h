#pragma once

#include "Platform.h"

#include <filesystem>

#if defined(BC_PLATFORM_WINDOWS)

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <shlobj.h>
#include <atlstr.h>
#include <commdlg.h>

#elif defined(BC_PLATFORM_LINUX)

#include "tinyfiledialogs.h"

#endif

#include <GLFW/glfw3.h>

namespace BC::Util
{
    static std::string NormaliseFilePathToString(const std::filesystem::path& file_path)
    {
        return file_path.generic_string();
    }

    static bool FilePathContains(const std::filesystem::path& file_path, const std::string& contain_string)
    {
        return file_path.generic_string().find(contain_string) != std::string::npos;
    }
    
    static bool IsPathRelativeTo(const std::filesystem::path& target, const std::filesystem::path& base)
    {
        auto target_canonical = std::filesystem::weakly_canonical(target);
        auto base_canonical   = std::filesystem::weakly_canonical(base);

        auto rel = std::filesystem::relative(target_canonical, base_canonical);

        // If target is not inside base, relative() will go up (e.g., "../..")
        return !rel.empty() && *rel.begin() != "..";
    }

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

    static std::string OpenFile(const char* filter, const std::filesystem::path& initial_dir = "")
    {

    #if defined(BC_PLATFORM_WINDOWS)

        OPENFILENAMEA ofn;
        CHAR szFile[260] = { 0 };
        ZeroMemory(&ofn, sizeof(OPENFILENAME));
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::GetWindow()->GetNativeWindow());
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);

        std::string initial_dir_str;

        // If initial_dir is not passed (empty string), use current directory
        if (initial_dir.empty())
        {
            CHAR currentDir[256] = { 0 };
            if (GetCurrentDirectoryA(256, currentDir))
                ofn.lpstrInitialDir = currentDir;
        }
        else
        {
            // If initial_dir is passed, validate and use it if it's a valid directory
            if (std::filesystem::exists(initial_dir.lexically_normal()) && std::filesystem::is_directory(initial_dir.lexically_normal()))
            {
                initial_dir_str = initial_dir.lexically_normal().string(); // Store path in a string
                ofn.lpstrInitialDir = initial_dir_str.c_str(); // Use the valid string
            }
            else
            {
                // Fallback to current directory if the provided path is invalid
                CHAR currentDir[256] = { 0 };
                if (GetCurrentDirectoryA(256, currentDir))
                    ofn.lpstrInitialDir = currentDir;
            }
        }

        ofn.lpstrFilter = filter;
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

        if (GetOpenFileNameA(&ofn) == TRUE)
            return ofn.lpstrFile;

        return std::string();

    #elif defined (BC_PLATFORM_LINUX)

        const char* path = tinyfd_openFileDialog("Open File", initial_dir.empty() ? nullptr : initial_dir.string().c_str(), 0, nullptr, nullptr, 0);
        return path ? std::string(path) : std::string();

    #else

        return "";
        
    #endif

    }

    static std::string OpenDirectory()
    {

    #if defined(BC_PLATFORM_WINDOWS)

        // Initialize COM
        CoInitialize(NULL);

        // Create the IFileOpenDialog object
        IFileOpenDialog* pFileOpen = nullptr;
        HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_PPV_ARGS(&pFileOpen));

        if (FAILED(hr))
        {
            CoUninitialize();
            return std::string(); // Return empty string on failure
        }

        // Set the dialog to pick folders
        DWORD dwFlags;
        pFileOpen->GetOptions(&dwFlags);
        pFileOpen->SetOptions(dwFlags | FOS_PICKFOLDERS); // Add the folder picking option

        // Show the dialog
        hr = pFileOpen->Show(NULL);
        if (FAILED(hr))
        {
            pFileOpen->Release();
            CoUninitialize();
            return std::string(); // Return empty string if dialog fails
        }

        // Retrieve the selected folder
        IShellItem* pItem = nullptr;
        hr = pFileOpen->GetResult(&pItem);
        if (FAILED(hr))
        {
            pFileOpen->Release();
            CoUninitialize();
            return std::string(); // Return empty string if no result
        }

        // Get the folder path
        PWSTR pszFolderPath = nullptr;
        hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFolderPath);
        if (FAILED(hr))
        {
            pItem->Release();
            pFileOpen->Release();
            CoUninitialize();
            return std::string(); // Return empty string if path retrieval fails
        }

        // Convert the WCHAR path to std::string using WideCharToMultiByte
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, pszFolderPath, -1, NULL, 0, NULL, NULL);
        std::string strFolderPath(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, pszFolderPath, -1, &strFolderPath[0], size_needed, NULL, NULL);


        // Clean up
        CoTaskMemFree(pszFolderPath);
        pItem->Release();
        pFileOpen->Release();
        CoUninitialize();

        return strFolderPath; // Return the selected folder path

    #elif defined (BC_PLATFORM_LINUX)

        const char* path = tinyfd_selectFolderDialog("Select Folder", nullptr);
        return path ? std::string(path) : std::string();

    #else

        return "";

    #endif

    }

    static std::string SaveFile(const char* filter, const std::filesystem::path& initial_dir = "")
    {

    #if defined(BC_PLATFORM_WINDOWS)

        OPENFILENAMEA ofn;
        CHAR szFile[260] = { 0 };
        ZeroMemory(&ofn, sizeof(OPENFILENAME));
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::GetWindow()->GetNativeWindow());
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);

        std::string initial_dir_str;

        // If initial_dir is not passed (empty string), use current directory
        if (initial_dir.empty())
        {
            CHAR currentDir[256] = { 0 };
            if (GetCurrentDirectoryA(256, currentDir))
                ofn.lpstrInitialDir = currentDir;
        }
        else
        {
            // If initial_dir is passed, validate and use it if it's a valid directory
            if (std::filesystem::exists(initial_dir.lexically_normal()) && std::filesystem::is_directory(initial_dir.lexically_normal()))
            {
                initial_dir_str = initial_dir.lexically_normal().string(); // Store path in a string
                ofn.lpstrInitialDir = initial_dir_str.c_str(); // Use the valid string
            }
            else
            {
                // Fallback to current directory if the provided path is invalid
                CHAR currentDir[256] = { 0 };
                if (GetCurrentDirectoryA(256, currentDir))
                    ofn.lpstrInitialDir = currentDir;
            }
        }

        ofn.lpstrFilter = filter;
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

        // Sets the default extension by extracting it from the filter
        ofn.lpstrDefExt = strchr(filter, '\0') + 1;

        if (GetSaveFileNameA(&ofn) == TRUE)
            return ofn.lpstrFile;

        return std::string();

    #elif defined(BC_PLATFORM_LINUX)

        const char* path = tinyfd_saveFileDialog("Save File", initial_dir.empty() ? nullptr : initial_dir.string().c_str(), 0, nullptr, nullptr);
        return path ? std::string(path) : std::string();

    #else

        return "";

    #endif

    }
}