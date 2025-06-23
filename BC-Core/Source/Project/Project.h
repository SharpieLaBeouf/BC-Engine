#pragma once

// Core Headers
#include "Asset/AssetManager.h"

#include "Scene/SceneManager.h"

// C++ Standard Library Headers
#include <memory>
#include <filesystem>

// External Vendor Library Headers

namespace BC
{

    class Project
    {
    
    public:

        Project();
        ~Project();

        Project(const Project& other) = default;
        Project(Project&& other) = default;

        Project& operator=(const Project& other) = default;
        Project& operator=(Project&& other) = default;

        SceneManager* GetSceneManager() const { return m_SceneManager.get(); }
        AssetManagerBase* GetAssetManager() const { return m_AssetManager.get(); }
        
        std::filesystem::path GetDirectory() const { return "Sandbox Project"; }

    private:

        std::unique_ptr<SceneManager> m_SceneManager = std::make_unique<SceneManager>();
        std::unique_ptr<AssetManagerBase> m_AssetManager = nullptr;

    };
    
}