#pragma once

// Core Headers
#include "Core/GUID.h"

#include "Project/Scene/Bounds/Octree.h"

// C++ Standard Library Headers
#include <string>
#include <memory>
#include <filesystem>
#include <unordered_map>

// External Vendor Library Headers
#include <entt/entt.hpp>

namespace BC
{
    class Entity;

    class Scene
    {
    
    public:

        Scene();
        Scene(const std::filesystem::path& scene_file_path);
        ~Scene();

        Scene(const Scene& other);
        Scene(Scene&& other);

        Scene& operator=(const Scene& other);
        Scene& operator=(Scene&& other);

		Entity CreateEntity(const std::string& name = "", GUID parent_guid = NULL_GUID);
		Entity CreateEntity(GUID entity_guid, const std::string& name = "", GUID parent_guid = NULL_GUID);

		// TODO: Entity InstantiatePrefab(std::shared_ptr<Prefab> prefab, std::optional<TransformComponent> transform = std::nullopt, const GUID& parent_uuid = NULL_GUID);

        void DuplicateEntity(const Entity& entity);

        void DestroyEntity(const Entity& entity);
        
		entt::registry* GetRegistry() { return &m_Registry; }
        
		template<typename... Components>
		auto GetAllEntitiesWith() {	return m_Registry.view<Components...>(); }

        Entity GetEntity(GUID entity_guid) const;
        Entity GetEntity(const std::string& entity_name) const;

        bool HasEntity(GUID entity_guid) const;
        bool HasEntity(const std::string& entity_name) const;

        bool IsEntityValid(const Entity& entity) const;

        void OnUpdate();
        void OnFixedUpdate();
        void OnLateUpdate();
        
        std::shared_ptr<OctreeBounds<Entity>> GetOctree() { return m_Octree; }

        GUID GetSceneID() const { return m_SceneID; }
        const std::string& GetName() const { return m_SceneName; }

        void MarkHierarchyDirty() { m_HierarchyChangedThisFrame.store(true); }
        bool IsHierarchyDirty() const { return m_HierarchyChangedThisFrame.load(); }

        void SaveScene();

    private:

        void Serialise();
        void Deserialise(const std::filesystem::path& scene_file_path);

        /// @brief This creates a generic default scene
        static std::shared_ptr<Scene> CreateDefaultScene(const std::filesystem::path& scene_file_path);

        /// @brief This will load a scene from a file and return a handle to
        /// that scene. Path must be relative to Project/Scenes folder. Option
        /// to provide manual project directory.
        static std::shared_ptr<Scene> LoadScene(const std::filesystem::path& scene_file_path, const std::filesystem::path& project_directory = "");

        /// @brief This will copy the source scene and return a dest scene.
        ///
        /// For Physics Components, it will not make copies of the instances,
        /// rather it will only hold onto the information of the physics
        /// components so that when dest scene is used it can initialise its own
        /// instances.
        static std::shared_ptr<Scene> CopyScene(std::shared_ptr<Scene> source_scene);

    private:
    
        /// @brief The Scene ID assigned when created for the first time
        GUID m_SceneID = NULL_GUID;

        /// @brief The name of the scene
        std::string m_SceneName = "Untitled Scene";

        /// @brief This is relative to the Project/Scenes folder
        std::filesystem::path m_SceneFilePath = "Untitled Scene.scene";

        entt::registry m_Registry;
        std::unordered_map<GUID, entt::entity> m_EntityMap = {};

		std::shared_ptr<OctreeBounds<Entity>> m_Octree = nullptr;

        /// @brief Used as a dirty flag to indicate to Editor's Hierarchy Panel if the state of the hierarchy has changed and its references will need to change
        std::atomic<bool> m_HierarchyChangedThisFrame = false;

        friend class Entity;
        friend class Project;
        friend class SceneManager;

    };
    
}