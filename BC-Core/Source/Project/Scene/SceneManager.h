#pragma once

// Core Headers
#include "Scene.h"
#include "Entity.h"

// C++ Standard Library Headers
#include <memory>
#include <vector>

// External Vendor Library Headers

namespace BC
{

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

        Entity GetEntity(GUID entity_guid) const;
        Entity GetEntity(const std::string& entity_name) const;

        bool HasEntity(GUID entity_guid) const { return GetEntity(entity_guid); }
        bool HasEntity(const std::string& entity_name) const { return GetEntity(entity_name); }

        bool IsEntityValid(const Entity& entity) const;

        void DestroyEntity(GUID entity_guid) const;
        void DestroyEntity(const std::string& entity_name) const;
        
        // ----------------------------
        //     Scene Functionality
        // ----------------------------

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

        const std::vector<std::shared_ptr<Scene>>& GetActiveScenes() const { return m_ActiveScenes; }

        template<typename... Components>
        std::vector<Entity> GetAllEntitiesWithComponent() const
        {
            std::vector<Entity> result;
            for (const auto& scene : m_ActiveScenes)
            {
                auto scene_view = scene->m_Registry.view<Components...>();
                for (const auto& entity_handle : scene_view)
                {
                    result.emplace_back(entity_handle, scene.get());
                }
            }
            return result;
        }

    private:

        bool m_IsRunning = false;
        bool m_IsSimulating = false;
        bool m_IsPaused = false;

        std::vector<std::shared_ptr<Scene>> m_ActiveScenes;

    };
    
}