#pragma once

// Core Headers
#include "Core/GUID.h"

#include "Project/Scene/Bounds/Octree.h"

// C++ Standard Library Headers
#include <memory>
#include <unordered_map>

// External Vendor Library Headers
#include <entt/entt.hpp>

namespace BC
{

    class Scene
    {
    
    public:

        Scene();
        ~Scene();

        Scene(const Scene& other);
        Scene(Scene&& other);

        Scene& operator=(const Scene& other);
        Scene& operator=(Scene&& other);
        
		entt::registry* GetRegistry() { return &m_Registry; }
        
		template<typename... Components>
		auto GetAllEntitiesWith() {	return m_Registry.view<Components...>(); }

        Entity GetEntity(GUID entity_guid) const;
        Entity GetEntity(const std::string& entity_name) const;

        bool HasEntity(GUID entity_guid) const;
        bool HasEntity(const std::string& entity_name) const;

        bool IsEntityValid(const Entity& entity) const;

        void DestroyEntity(const Entity& entity);

        void OnUpdate();
        void OnFixedUpdate();
        void OnLateUpdate();

    private:
    
        entt::registry m_Registry;
        std::unordered_map<GUID, entt::entity> m_EntityMap;

		std::shared_ptr<OctreeBounds<Entity>> m_Octree = nullptr;

        friend class Entity;
        friend class SceneManager;

    };
    
}