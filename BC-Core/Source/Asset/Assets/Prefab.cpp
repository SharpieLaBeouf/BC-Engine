#include "BC_PCH.h"
#include "Prefab.h"

namespace BC
{

#pragma region Prefab Entity
    
    PrefabEntity::PrefabEntity() :
        m_PrefabEntityHandle(entt::null),
        m_Prefab(nullptr)
    {

    }

    PrefabEntity::PrefabEntity(entt::entity handle, Prefab* prefab_ptr) :
        m_PrefabEntityHandle(handle),
        m_Prefab(prefab_ptr)
    {

    }

    PrefabEntity::operator bool() const { return m_Prefab ? m_Prefab->GetRegistry()->valid(m_PrefabEntityHandle) : false; }
    bool PrefabEntity::operator==(const PrefabEntity& other) const { return m_PrefabEntityHandle == other.m_PrefabEntityHandle && m_Prefab == other.m_Prefab; }

    template <typename T, typename... Args>
    T& PrefabEntity::AddComponent(Args &&...args)
    {
        if (m_PrefabEntityHandle == entt::null) 
        {
            BC_CORE_ERROR("PrefabEntity::AddComponent<{}>: Cannot Add Component as Entity Handle is Null.", typeid(T).name());
            return Entity::GetBlankComponent<T>();
        }

        if (HasComponent<T>())
        {
            BC_CORE_TRACE("PrefabEntity::AddComponent: Entity Already Has: {0}", typeid(T).name());
            return GetComponent<T>();
        }

        T& component = m_Prefab->m_Registry.emplace<T>(m_PrefabEntityHandle, std::forward<Args>(args)...);

        return component;
    }

    template<typename T>
    T& PrefabEntity::GetComponent() const
    {
        if (m_PrefabEntityHandle == entt::null) 
        {
            BC_CORE_ERROR("Entity::GetComponent<{}>: Cannot Get Component as Entity Handle is Null.", typeid(T).name());
            return Entity::GetBlankComponent<T>();
        }
        
        if (!HasComponent<T>()) 
        {
            BC_CORE_ERROR("Entity::GetComponent<{}>: Entity Does Not Have Component.");
            return Entity::GetBlankComponent<T>();
        }

        return m_Prefab->m_Registry.get<T>(m_PrefabEntityHandle);
    }

    template<typename T>
    T* PrefabEntity::TryGetComponent() const
    {
        if (m_PrefabEntityHandle == entt::null) 
        {
            BC_CORE_ERROR("PrefabEntity::TryGetComponent<{}>: Cannot Get Component as Entity Handle is Null.", typeid(T).name());
            return nullptr;
        }

        return m_Prefab->m_Registry.try_get<T>(m_PrefabEntityHandle);
    }

    template<typename T>
    void PrefabEntity::RemoveComponent()
    {
        if (!HasComponent<T>()) 
        {
            return;
        }

        m_Prefab->m_Registry.remove<T>(m_PrefabEntityHandle);
    }
    
    template <typename T>
    bool PrefabEntity::HasComponent() const { return m_Prefab->m_Registry.all_of<T>(m_PrefabEntityHandle); }

    template<typename... Components>
    bool PrefabEntity::HasAnyComponent() const { return m_Prefab->m_Registry.any_of<Components...>(m_PrefabEntityHandle); }

#define INSTANTIATE_PREFAB_ENTITY_COMPONENT(T) \
    template T& PrefabEntity::AddComponent<T>(); \
    template T& PrefabEntity::GetComponent<T>() const; \
    template T* PrefabEntity::TryGetComponent<T>() const; \
    template void PrefabEntity::RemoveComponent<T>(); \
    template bool PrefabEntity::HasComponent<T>() const; \
    template bool PrefabEntity::HasAnyComponent<T>() const; \

EXPAND_COMPONENTS(INSTANTIATE_PREFAB_ENTITY_COMPONENT);

#pragma endregion

#pragma region Prefab Entity

    PrefabEntity Prefab::GetEntity(GUID prefab_entity_guid) const
    {
        if (m_EntityMap.contains(prefab_entity_guid))
        {
            return PrefabEntity{ m_EntityMap.at(prefab_entity_guid), const_cast<Prefab*>(this) };
        }

        return PrefabEntity{};
    }

    PrefabEntity Prefab::GetEntity(const std::string& prefab_entity_name) const
    {
        auto view = m_Registry.view<MetaComponent>();
        for (auto entity_handle : view) 
        {
            const MetaComponent& meta_component = view.get<MetaComponent>(entity_handle);
            if (meta_component.GetName() == prefab_entity_name)
                return PrefabEntity { entity_handle, const_cast<Prefab*>(this) };
        }

        return PrefabEntity{};
    }

    void Prefab::SetRootEntity(const PrefabEntity &prefab_root_entity)
    {
        if (!prefab_root_entity)
            return;

        m_RootEntity = prefab_root_entity;
    }

    PrefabEntity Prefab::GetRootEntity() const { return m_RootEntity; }

    bool Prefab::HasEntity(GUID prefab_entity_guid) const { return GetEntity(prefab_entity_guid).operator bool(); }

    bool Prefab::HasEntity(const std::string& prefab_entity_name) const { return GetEntity(prefab_entity_name).operator bool(); }

    bool Prefab::IsEntityValid(const PrefabEntity &prefab_entity) const { return m_Registry.valid(static_cast<entt::entity>(prefab_entity)); }

#pragma endregion

#pragma region Prefab Entity Static Methods

    // TODO: Implement
    std::shared_ptr<Prefab> Prefab::CopyPrefab(std::shared_ptr<Prefab> source_prefab)
    {
        return std::shared_ptr<Prefab>();
    }

    // TODO: Implement
    std::shared_ptr<Prefab> Prefab::CreatePrefabFromSerialisedData(const std::string &serialised_prefab_data)
    {
        return std::shared_ptr<Prefab>();
    }

#pragma endregion

}