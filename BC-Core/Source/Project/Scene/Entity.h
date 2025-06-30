#pragma once

// Core Headers
#include "Scene.h"

#include "Core/GUID.h"

#include "Components/CoreComponents.h"
#include "Components/CameraComponents.h"
#include "Components/AudioComponents.h"
#include "Components/MeshComponents.h"
#include "Components/AnimationComponents.h"
#include "Components/LightComponents.h"
#include "Components/PhysicsComponents.h"

// C++ Standard Library Headers
#include <memory>
#include <typeindex>
#include <unordered_map>

// External Vendor Library Headers
#include <entt/entt.hpp>

namespace BC
{

    class Entity
    {
    
    public:

        Entity();
		Entity(entt::entity handle, Scene* scene_ptr);
        ~Entity() = default;

		Entity(const Entity&) = default;
		Entity(Entity&&) = default;

		Entity& operator=(const Entity&) = default;
		Entity& operator=(Entity&&) = default;

#pragma region Setters

		// @brief This adds a Component to the applicable Entity, and returns that Component
		template<typename T, typename... Args>
		T& AddComponent(Args&&... args);
        
		/// @brief This removes any Component within the applicable Entity
		template <typename T>
		void RemoveComponent();

		void Destroy();

#pragma endregion

#pragma region Getters

		template<typename T>
		T& GetComponent() const;

		template<typename T>
		T* TryGetComponent() const;
        
		template<typename T>
		T& GetComponentInParent() const;
        
		template<typename T>
		T& GetComponentInChild() const;
        
		template<typename T>
		std::vector<Entity> GetComponentsInParents() const;
        
		template<typename T>
		std::vector<Entity> GetComponentsInChildren() const;
        
		template<typename T>
		std::vector<Entity> GetComponentsInSelfAndChildren() const;

        GUID GetGUID() const { return GetComponent<MetaComponent>().GetEntityGUID(); }
		const std::string& GetName() const { return GetComponent<MetaComponent>().GetName(); }
        TransformComponent& GetTransform() const { return GetComponent<TransformComponent>(); }

		operator entt::entity() const { return m_EntityHandle; }
		operator uint32_t() const { return static_cast<uint32_t>(m_EntityHandle); }

		Scene* GetScene() const { return m_Scene; }

#pragma endregion

#pragma region Validation

		template <typename T>
		bool HasComponent() const { return m_Scene->m_Registry.all_of<T>(m_EntityHandle); }

		template<typename... Components>
		bool HasAnyComponent() const { return m_Scene->m_Registry.any_of<Components...>(m_EntityHandle); }

		operator bool() const { return m_Scene ? m_Scene->GetRegistry()->valid(m_EntityHandle) : false; }
		bool operator==(const Entity& other) const { return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene; }

#pragma endregion

		template<typename T>
		static T& GetBlankComponent()
		{
			static std::unordered_map<std::type_index, std::shared_ptr<void>> s_BlankComponents;

			auto it = s_BlankComponents.find(typeid(T));
			if (it == s_BlankComponents.end())
			{
				s_BlankComponents[typeid(T)] = std::make_shared<T>();
			}

			return *std::static_pointer_cast<T>(s_BlankComponents[typeid(T)]);
		}

    private:
    
		Scene* m_Scene = nullptr;
        entt::entity m_EntityHandle;

		friend struct ComponentBase;
    };
    
}

namespace std
{
	template<>
	struct hash<BC::Entity>
	{
		std::size_t operator()(const BC::Entity& entity) const noexcept
		{
			return std::hash<uint64_t>()(
				(static_cast<uint64_t>(static_cast<uint32_t>(static_cast<entt::entity>(entity))) << 32) ^
				reinterpret_cast<uintptr_t>(entity.GetScene()));
		}
	};
}