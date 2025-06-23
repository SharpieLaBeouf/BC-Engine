#include "BC_PCH.h"
#include "Scene.h"
#include "Entity.h"

namespace BC
{

#pragma region Scene Constructors

    Scene::Scene()
    {

    }

    Scene::~Scene()
    {

    }

    Scene::Scene(const Scene &other)
    {

    }
    
    Scene::Scene(Scene&& other)
    {

    }
    
    Scene& Scene::operator=(const Scene &other)
    {
        if (this == &other)
            return *this;

        // TODO: Implement

        return *this;
    }
    
    Scene& Scene::operator=(Scene&& other)
    {
        if (this == &other)
            return *this;

        // TODO: Implement

        return *this;
    }

#pragma endregion

#pragma region Methods

    Entity Scene::GetEntity(GUID entity_guid) const
    {
        if (m_EntityMap.contains(entity_guid))
        {
            return Entity{ m_EntityMap.at(entity_guid), const_cast<Scene*>(this) };
        }

        return Entity{};
    }

    Entity Scene::GetEntity(const std::string &entity_name) const
    {
        auto view = m_Registry.view<MetaComponent>();
        for (auto entity_handle : view) 
        {
            const MetaComponent& meta_component = view.get<MetaComponent>(entity_handle);
            if (meta_component.GetName() == entity_name)
                return Entity { entity_handle, const_cast<Scene*>(this) };
        }

        return Entity{};
    }

    bool Scene::HasEntity(GUID entity_guid) const { return GetEntity(entity_guid); }

    bool Scene::HasEntity(const std::string& entity_name) const { return GetEntity(entity_name); }

    bool Scene::IsEntityValid(const Entity& entity) const
    {
        return m_Registry.valid(static_cast<entt::entity>(entity));
    }

    static void DestroyEntityHelper(const Entity& entity, std::unordered_map<GUID, entt::entity>& entity_map)
    {
        Scene* scene = entity.GetScene();
        entt::registry* registry = entity.GetScene()->GetRegistry();

        if (!entity || !registry->valid(static_cast<entt::entity>(entity)))
        {
            BC_CORE_WARN("Scene::DestroyEntity: Could Not Destroy Invalid Entity.");
            return;
        }

        // TODO: implement destroy logic
		// if (Application::GetProject()->GetSceneManager()->IsRunning() && entity.GetComponent<MetaComponent>().HasScripts())
		// 	Application::GetProject()->GetSceneManager()->OnDestroyAllScripts(entity.GetGUID());

        auto& component = entity.GetComponent<MetaComponent>();

        if (component.HasParent())
            component.DetachParent();

        // Make a copy of the child list so we don't invalidate 
        // the iterator whilst destroying children
        std::vector<GUID> children = component.GetChildrenGUID();
        for (const auto& child_guid : children)
            DestroyEntityHelper(entity.GetScene()->GetEntity(child_guid), entity_map);
		
        // Destroy this entity
		entity_map.erase(entity.GetGUID());

		// 6. Destroy the Entity and Components from the ENTT Registry
		registry->destroy(entity);

    }
    
    void Scene::DestroyEntity(const Entity& entity)
    {
        // Obtain lock
		std::unique_lock<std::mutex> scene_octree_lock(m_Octree->GetOctreeMutex());

        DestroyEntityHelper(entity, m_EntityMap);

    }
    
#pragma endregion

    void Scene::OnUpdate()
    {

    }
    
    void Scene::OnFixedUpdate()
    {

    }
    
    void Scene::OnLateUpdate()
    {

    }

}