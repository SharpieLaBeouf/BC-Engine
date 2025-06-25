#pragma once

// Core Headers
#include "Core/GUID.h"

#include "Scene.h"
#include "Entity.h"

#include "Physics/PhysicsSystem.h"

// C++ Standard Library Headers
#include <memory>
#include <filesystem>
#include <unordered_map>

// External Vendor Library Headers

namespace physx
{
    class PxScene;
}

namespace YAML
{
    class Emitter;
    class Node;
}

using namespace physx;

namespace BC
{

    class CollisionCallback;

    class SceneManager
    {
    
    public:

        SceneManager();
        ~SceneManager();

        SceneManager(const SceneManager& other) = default;
        SceneManager(SceneManager&& other) = default;

        SceneManager& operator=(const SceneManager& other) = default;
        SceneManager& operator=(SceneManager&& other) = default;

        // ----------------------------
        //          Entities
        // ----------------------------

        #pragma region Entities

        Entity GetEntity(GUID entity_guid) const;
        Entity GetEntity(const std::string& entity_name) const;

        bool HasEntity(GUID entity_guid) const { return GetEntity(entity_guid); }
        bool HasEntity(const std::string& entity_name) const { return GetEntity(entity_name); }

        bool IsEntityValid(const Entity& entity) const;

        void DestroyEntity(GUID entity_guid) const;
        void DestroyEntity(const std::string& entity_name) const;
        
        #pragma endregion

        // ----------------------------
        //     Scene Functionality
        // ----------------------------

        #pragma region Scene Functionality

        void OnStart();
        void OnStop();

        void OnStartRuntime();
        void OnStopRuntime();

        void OnStartSimulation();
        void OnStopSimulation();

        void OnStartPhysics();
        void OnStopPhysics();

		void OnUpdate();
		void OnFixedUpdate();
		void OnLateUpdate();

        bool IsRunning() const { return m_IsRunning; }
        void SetRunning(bool running) { m_IsRunning = running; }
        
        bool IsSimulating() const { return m_IsSimulating; }
        void SetSimulating(bool simulating) { m_IsSimulating = simulating; }

        bool IsPaused() const { return m_IsPaused; }
        void SetPaused(bool paused) { m_IsPaused = paused; }
        
        PhysicsSystem* GetPhysicsSystem() const { return m_PhysicsSystem.get(); }

        std::unordered_map<GUID, std::shared_ptr<Scene>>& GetSceneInstances() { return m_SceneInstances; }
        std::unordered_map<GUID, std::filesystem::path>& GetSceneFilePaths() { return m_SceneFilePaths; }
        std::shared_ptr<Scene> GetPersistentScene() { return m_PersistentScene; }

        template<typename... Components>
        std::vector<Entity> GetAllEntitiesWithComponent() const
        {
            std::vector<Entity> result;
            for(auto [scene_id, scene] : m_SceneInstances)
            {
                auto scene_view = scene->m_Registry.view<Components...>();
                for (const auto& entity_handle : scene_view)
                {
                    result.emplace_back(entity_handle, scene.get());
                }
            }
            return result;
        }

        #pragma endregion

        // ----------------------------
        //       Scene Manager
        // ----------------------------

        #pragma region Scene Manager

        void LoadScene(const std::string& scene_name, bool additive = false);
        void LoadSceneAsync(const std::string& scene_name, bool additive = false);
        
        void LoadScene(GUID scene_guid, bool additive = false, const std::filesystem::path& project_directory = "");
        void LoadSceneAsync(GUID scene_guid, bool additive = false, const std::filesystem::path& project_directory = "");

        /// @brief This will load a scene into the SceneManager which is not included in m_SceneFilepaths
        ///
        /// For example, for the editor, you may load scenes for editing, but
        /// not add them to the m_SceneFilePaths (e.g., what is included in
        /// final build)
        void LoadSceneNoAdd(const std::filesystem::path& scene_file_path, bool additive = false);
        
        /// @brief This will load a scene into the SceneManager which is not included in m_SceneFilepaths
        ///
        /// For example, for the editor, you may load scenes for editing, but
        /// not add them to the m_SceneFilePaths (e.g., what is included in
        /// final build)
        void LoadSceneAsyncNoAdd(const std::filesystem::path& scene_file_path, bool additive = false);

        void AddSceneTemplate(std::shared_ptr<Scene> scene);
        void AddSceneTemplate(GUID scene_id, const std::filesystem::path& scene_file_path);

        void SetActiveScene(GUID scene_id);
        void SetActiveScene(const std::string& scene_name);

        void SetEntryScene(GUID scene_id);
        void SetEntryScene(const std::string& scene_name);
        GUID GetEntryScene() const { return m_EntryScene; }

        void UnloadScene(GUID scene_guid);
        void UnloadScene(const std::string& scene_name);

        std::shared_ptr<Scene> GetActiveScene();

        void Serialise(YAML::Emitter& out);
        void Deserialise(const YAML::Node& data, const std::filesystem::path& project_directory);

        void SaveAllScenes();

        #pragma endregion

    private:

        bool m_IsRunning = false;
        bool m_IsSimulating = false;
        bool m_IsPaused = false;

        // TODO: Make Scene Manager Hold "Unloaded Scenes" and when Loaded, it
        // will deserialise from file, Unloaded scenes will just be relative
        // file paths to scene file in project/scenes

        /// @brief This holds instances of scenes that are currently simulating/running
        std::unordered_map<GUID, std::shared_ptr<Scene>> m_SceneInstances = {};

        /// @brief This holds relative paths to all scenes in Project/Scenes folder
        std::unordered_map<GUID, std::filesystem::path> m_SceneFilePaths = {};

        /// @brief This is the reference to the Target Scene for New Entities and Entity Instantiations
        GUID m_ActiveScene = NULL_GUID;

        /// @brief This is the Scene ID of the Scene that will be instantiated on start
        GUID m_EntryScene = NULL_GUID;

        /// @brief To be initialised when SceneManager runtime starts
        std::shared_ptr<Scene> m_PersistentScene = nullptr;

        /// @brief To be initialised when SceneManager simulation/runtime starts
        std::unique_ptr<PhysicsSystem> m_PhysicsSystem = nullptr;

        friend class Scene;
        friend class Project;

    };
    
}