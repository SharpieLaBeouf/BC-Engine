#include "BC_PCH.h"
#include "Entity.h"
#include "Scene.h"

#include "Core/Application.h"

namespace BC
{
    Entity::Entity() :
        m_EntityHandle(entt::null),
        m_Scene(nullptr)
    {

    }

    Entity::Entity(entt::entity handle, Scene* scene_ptr) :
        m_EntityHandle(handle),
        m_Scene(scene_ptr)
    {

    }
    
    void Entity::Destroy()
    {
        
    }

#pragma region Add/Remove Components

    template<typename T, typename... Args>
    T& Entity::AddComponent(Args&&... args) 
    {
        if (HasComponent<T>()) 
        {
            BC_CORE_TRACE("Entity Already Has: {0}", typeid(T).name());
            return GetComponent<T>();
        }

        T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
        component.SetEntity(*this);
        component.Init();

        return component;
    }

    template <typename T>
    void Entity::RemoveComponent() 
    {
        if (!HasComponent<T>()) 
        {
            return;
        }

        GetComponent<T>().Shutdown();
        
        // Ensure the Octree Job Does Not Access Component After Removed
        if constexpr (std::is_same_v<T, MeshRendererComponent> || std::is_same_v<T, SkinnedMeshRendererComponent>)
        {
            std::unique_lock<std::mutex> scene_octree_lock(m_Scene->m_Octree->GetOctreeMutex());
            m_Scene->m_Registry.remove<T>(m_EntityHandle);
            return;
        }

        m_Scene->m_Registry.remove<T>(m_EntityHandle);

        return;
    }

#pragma endregion

#pragma region Get Components

    template<typename T>
    T& Entity::GetComponent() const
    {
        if (m_EntityHandle == entt::null) 
        {
            BC_CORE_ERROR("Entity::GetComponent<{}>: Cannot Get Component as Entity Handle is Null.", typeid(T).name());
            return GetBlankComponent<T>();
        }
        
        if (!HasComponent<T>()) 
        {
            BC_CORE_ERROR("Entity::GetComponent<{}>: Entity Does Not Have Component.");
            return GetBlankComponent<T>();
        }

        return m_Scene->m_Registry.get<T>(m_EntityHandle);
    }

    template<typename T>
    T* Entity::TryGetComponent() const
    {
        if (m_EntityHandle == entt::null) 
        {
            BC_CORE_ERROR("Entity::TryGetComponent<{}>: Cannot Get Component as Entity Handle is Null.", typeid(T).name());
            return nullptr;
        }

        return m_Scene->m_Registry.try_get<T>(m_EntityHandle);
    }
        
    template<typename T>
    T& Entity::GetComponentInParent() const
    {
        std::function<T&(Entity)> recursive_parent_search;
        recursive_parent_search = [&](const Entity& current_entity) -> T&
        {
            // If the current entity has the component, return it
            if (current_entity.HasComponent<T>())
            {
                return current_entity.GetComponent<T>();
            }

            BC_ASSERT(current_entity.HasComponent<MetaComponent>(), std::format("Entity::GetComponentInParent<{}>: Entity Does Not Have MetaData Component - All Entities Must Have This Component.", typeid(T).name()));
            MetaComponent& meta_component = current_entity.GetComponent<MetaComponent>();
            const Entity& parent_entity = Application::GetProject()->GetSceneManager()->GetEntity(meta_component.GetParentGUID());
            if (parent_entity)  // If a parent exists, continue searching
            {
                return recursive_parent_search(parent_entity);
            }

            return GetBlankComponent<T>();
        };

        BC_ASSERT(HasComponent<MetaComponent>(), std::format("Entity::GetComponentInParent<{}>: Entity Does Not Have MetaData Component - All Entities Must Have This Component.", typeid(T).name()));
        MetaComponent& meta_component = GetComponent<MetaComponent>();
        const Entity& parent_entity = Application::GetProject()->GetSceneManager()->GetEntity(meta_component.GetParentGUID());
        if (parent_entity)  // If a parent exists, continue searching
        {
            return recursive_parent_search(parent_entity);
        }

        return GetBlankComponent<T>();
    }
        
    template<typename T>
    T& Entity::GetComponentInChild() const
    {
        std::function<T& (Entity)> recursive_child_search;
        recursive_child_search = [&](const Entity&  current_entity) -> T&
        {
            BC_ASSERT(current_entity.HasComponent<MetaComponent>(), std::format("Entity::GetComponentInChild<{}>: Entity Does Not Have MetaData Component - All Entities Must Have This Component.", typeid(T).name()));
            const MetaComponent& meta_component = current_entity.GetComponent<MetaComponent>();

            if (meta_component.HasChildren())
            {
                const auto& children = meta_component.GetChildrenGUID();

                // 1. Check all direct children for the component
                for (const auto& child_guid : children)
                {
                    Entity child_entity = Application::GetProject()->GetSceneManager()->GetEntity(child_guid);
                    if (child_entity && child_entity.HasComponent<T>())
                    {
                        return child_entity.GetComponent<T>();
                    }
                }

                // 2. If not found, recurse into children
                for (const auto& child_guid : children)
                {
                    Entity child_entity = Application::GetProject()->GetSceneManager()->GetEntity(child_guid);
                    if (!child_entity) continue;  // Ensure entity is valid before recursing

                    T& found_component = recursive_child_search(child_entity);
                    if (&found_component != &GetBlankComponent<T>())  // Component found in a child
                    {
                        return found_component;
                    }
                }
            }

            return GetBlankComponent<T>();
        };

        return recursive_child_search(*this);
    }
        
    template<typename T>
    std::vector<Entity> Entity::GetComponentsInParents() const
    {
		std::vector<Entity> entities_with_component;

		std::function<void(Entity)> recursive_parent_search;
		recursive_parent_search = [&](const Entity& current_entity)
		{
			if (current_entity.HasComponent<T>())
			{
				entities_with_component.push_back(current_entity);
			}

			BC_ASSERT(current_entity.HasComponent<MetaComponent>(), std::format("Entity::GetComponentsInParents<{}>: Entity Does Not Have MetaData Component - All Entities Must Have This Component.", typeid(T).name()));
			MetaComponent& meta_component = current_entity.GetComponent<MetaComponent>();
        	const Entity& parent_entity = Application::GetProject()->GetSceneManager()->GetEntity(meta_component.GetParentGUID());
			if (parent_entity)
			{
				recursive_parent_search(parent_entity);
			}
		};

		BC_ASSERT(HasComponent<MetaComponent>(), std::format("Entity::GetComponentsInParents<{}>: Entity Does Not Have MetaData Component - All Entities Must Have This Component.", typeid(T).name()));
		MetaComponent& meta_component = GetComponent<MetaComponent>();
		const Entity& parent_entity = Application::GetProject()->GetSceneManager()->GetEntity(meta_component.GetParentGUID());
		if (parent_entity)
		{
			recursive_parent_search(parent_entity);
		}

		return entities_with_component;
    }
    
    template<typename T>
    std::vector<Entity> Entity::GetComponentsInChildren() const
    {
		std::vector<Entity> entities_with_component;
		std::function<void(Entity)> recursive_child_search;

		recursive_child_search = [&](Entity current_entity)
		{
			BC_ASSERT(HasComponent<MetaComponent>(), std::format("Entity::GetComponentsInChildren<{}>: Entity Does Not Have MetaData Component - All Entities Must Have This Component.", typeid(T).name()));
			const MetaComponent& meta_component = current_entity.GetComponent<MetaComponent>();

			if (meta_component.HasChildren())
			{
				const auto& children = meta_component.GetChildrenGUID();

				// 1. Check all direct children for the component
				for (const auto& child_guid : children)
				{
					Entity child_entity = Application::GetProject()->GetSceneManager()->GetEntity(child_guid);
					if (child_entity && child_entity.HasComponent<T>())
					{
						entities_with_component.push_back(child_entity);
					}
				}

				// 2. Recurse into children
				for (const auto& child_guid : children)
				{
					Entity child_entity = Application::GetProject()->GetSceneManager()->GetEntity(child_guid);
					if (child_entity)
					{
						recursive_child_search(child_entity);
					}
				}
			}
		};

		recursive_child_search(*this);
		return entities_with_component;
    }
    
    template<typename T>
    std::vector<Entity> Entity::GetComponentsInSelfAndChildren() const
    {
		std::vector<Entity> entities_with_component;

		if (this->HasComponent<T>()) entities_with_component.push_back(*this);

		auto children = this->GetComponentsInChildren<T>();
		entities_with_component.insert(entities_with_component.end(), children.begin(), children.end());
		return entities_with_component;
    }

#pragma endregion

#pragma region Entity Function Template Specialisations

#define INSTANTIATE_ENTITY_COMPONENT(T) \
    template T& Entity::AddComponent<T>(); \
    template void Entity::RemoveComponent<T>(); \
    template T& Entity::GetComponent<T>() const; \
    template T* Entity::TryGetComponent<T>() const; \
    template T& Entity::GetComponentInParent<T>() const; \
    template T& Entity::GetComponentInChild<T>() const; \
    template std::vector<Entity> Entity::GetComponentsInParents<T>() const; \
    template std::vector<Entity> Entity::GetComponentsInChildren<T>() const; \
    template std::vector<Entity> Entity::GetComponentsInSelfAndChildren<T>() const;

EXPAND_COMPONENTS(INSTANTIATE_ENTITY_COMPONENT);

#pragma endregion

}