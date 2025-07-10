#pragma once

// Core Headers
#include "Asset/Asset.h"

// C++ Standard Library Headers
#include <string>

// External Vendor Library Headers
#include <entt/entt.hpp>

namespace BC
{
    using PrefabType = uint8_t;
    enum : PrefabType
    {
        PrefabType_Entity,
        PrefabType_Model
    };

    class Prefab;

    class PrefabEntity
    {
    
    public:

        PrefabEntity();
		PrefabEntity(entt::entity handle, Prefab* prefab_ptr);
        ~PrefabEntity() = default;

		PrefabEntity(const PrefabEntity&) = default;
		PrefabEntity(PrefabEntity&&) = default;

		PrefabEntity& operator=(const PrefabEntity&) = default;
		PrefabEntity& operator=(PrefabEntity&&) = default;

        GUID GetGUID() const { return GetComponent<MetaComponent>().GetEntityGUID(); }
		const std::string& GetName() const { return GetComponent<MetaComponent>().GetName(); }
        TransformComponent& GetTransform() const { return GetComponent<TransformComponent>(); }

        // Don't go through the MetaComponent hierarchy functions, we will set
        // manually as Prefabs are special cases separate from the Scene ECS

        void AttachParent(const PrefabEntity& prefab_entity_parent);
        void DetachParent();
        bool HasParent() const;
        bool HasChildren() const;

		operator entt::entity() const { return m_PrefabEntityHandle; }
		operator uint32_t() const { return static_cast<uint32_t>(m_PrefabEntityHandle); }

		operator bool() const;
		bool operator==(const PrefabEntity& other) const;

    public:

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args);

		template<typename T>
		T& GetComponent() const;

		template<typename T>
		T* TryGetComponent() const;

		template<typename T>
		void RemoveComponent();

		template <typename T>
		bool HasComponent() const;

		template<typename... Components>
		bool HasAnyComponent() const;

    private:
    
		Prefab* m_Prefab = nullptr;
        entt::entity m_PrefabEntityHandle;

		friend struct ComponentBase;
		friend class Prefab;
    };

    class Prefab : public Asset
    {
    
    public:

        Prefab() = default;
        ~Prefab() = default;

        Prefab(const Prefab& other) = delete;
        Prefab(Prefab&& other) = default;

        Prefab& operator=(const Prefab& other) = delete;
        Prefab& operator=(Prefab&& other) = default;
        
        /// @brief Used to determine if prefab is mutable origination from a .prefab file, or if it is immutable originating from a Model File e.g., FBX
        AssetType GetType() const override { return m_PrefabType == PrefabType_Model ? AssetType::PrefabModel : AssetType::PrefabEntity; }
        void SetPrefabType(PrefabType prefab_type) { m_PrefabType = prefab_type; }

		PrefabEntity CreateEntity(const std::string& prefab_entity_name = "", GUID prefab_parent_guid = NULL_GUID);
		PrefabEntity CreateEntity(GUID prefab_entity_guid, const std::string& prefab_entity_name = "", GUID prefab_parent_guid = NULL_GUID);

        const std::string& GetName() const { return m_Name; }
        void SetName(const std::string& prefab_name) { m_Name = prefab_name; }
        
		entt::registry* GetRegistry() { return &m_Registry; }

    public:
    
        template<typename... Components>
        auto GetAllEntitiesWith() { return m_Registry.view<Components...>(); }
        
        PrefabEntity GetEntity(GUID prefab_entity_guid) const;
        PrefabEntity GetEntity(const std::string& prefab_entity_name) const;
        
        void SetRootEntity(const PrefabEntity& prefab_root_entity);
        PrefabEntity GetRootEntity() const;
        
        bool HasEntity(GUID prefab_entity_guid) const;
        bool HasEntity(const std::string& prefab_entity_name) const;

        bool IsEntityValid(const PrefabEntity& prefab_entity) const;
    
    public:

        static std::shared_ptr<Prefab> CopyPrefab(std::shared_ptr<Prefab> source_prefab);

        static std::shared_ptr<Prefab> CreatePrefabFromSerialisedData(const std::string& serialised_prefab_data);

    private:

        std::string m_Name = "";

        PrefabType m_PrefabType = PrefabType_Entity;

        entt::registry m_Registry;
        
        PrefabEntity m_RootEntity = {};
        std::unordered_map<GUID, entt::entity> m_EntityMap = {};

        friend class PrefabEntity;

    };

}