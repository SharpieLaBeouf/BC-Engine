#pragma once

// Core Headers
#include "Asset/AssetManager.h"
#include "Scene/SceneManager.h"

#include "Util/Platform.h"

// C++ Standard Library Headers
#include <memory>
#include <filesystem>

// External Vendor Library Headers

namespace YAML 
{
	class Emitter;
	class Node;
}

namespace BC
{

    class Project
    {
    
    public:

        /// @brief This Will Create a Blank New Project With One Blank Scene Attached
        /// @param project_file_path This needs to be the file path of the serialised project file. The parent directory will be considered as the Project's root directory
        static std::unique_ptr<Project> NewProject(const std::filesystem::path& project_file_path, bool editor = false);

        /// @brief This Will Load an Existing Project if the File Path is Valid 
        /// @param project_file_path This needs to be the file path of the serialised project file. The parent directory will be considered as the Project's root directory
        static std::unique_ptr<Project> LoadProject(const std::filesystem::path& project_file_path, bool editor = false);

        Project();
        ~Project();

        Project(const Project& other) = default;
        Project(Project&& other) = default;

        Project& operator=(const Project& other) = default;
        Project& operator=(Project&& other) = default;

        SceneManager* GetSceneManager() const { return m_SceneManager.get(); }
        AssetManagerBase* GetAssetManager() const { return m_AssetManager.get(); }
        
        const std::string& GetName() const { return m_Name; }
        const std::filesystem::path& GetDirectory() const { return m_Directory; }

        void SaveProject();
        void ReloadProject();

        void Serialise() const;
        bool Deserialise(const std::filesystem::path& project_file_path);

    private:

        static bool ValidProjectFilePath(const std::filesystem::path& project_file_path);

    private:

        std::unique_ptr<AssetManagerBase> m_AssetManager = nullptr;
        
        std::unique_ptr<SceneManager> m_SceneManager = std::make_unique<SceneManager>();

        std::string m_Name = "Untitled Project";
        std::filesystem::path m_Directory = "Untitled Project";

    #if defined(BC_PLATFORM_WINDOWS)

        /// @brief Relative Path of script code binary
        const std::filesystem::path m_ScriptAssemblyPath = "Scripts/Bin/ScriptCode.dll";

    #elif defined(BC_PLATFORM_LINUX)

        /// @brief Relative Path of script code binary
        const std::filesystem::path m_ScriptAssemblyPath = "Scripts/Bin/ScriptCode.so";

    #endif

    };
    
}