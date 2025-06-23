#include "BC_PCH.h"
#include "SceneManager.h"

#include "Graphics/Renderer/SceneRenderer.h"

namespace BC
{
    SceneManager::SceneManager()
    {
    }

    SceneManager::~SceneManager()
    {
    }

    Entity SceneManager::GetEntity(GUID entity_guid) const
    {
        for (const auto& scene : m_ActiveScenes)
        {
            if (scene->m_EntityMap.contains(entity_guid))
                return Entity{ scene->m_EntityMap.at(entity_guid), scene.get() };
        }
        return Entity{};
    }

    Entity SceneManager::GetEntity(const std::string &entity_name) const
    {
        for (const auto& scene : m_ActiveScenes)
        {
            auto view = scene->m_Registry.view<MetaComponent>();
            for (auto entity_handle : view) 
            {
                const MetaComponent& meta_component = view.get<MetaComponent>(entity_handle);
                if (meta_component.GetName() == entity_name)
                    return Entity { entity_handle, scene.get() };
            }
        }
        return Entity{};
    }

    bool SceneManager::IsEntityValid(const Entity& entity) const
    {
        for (const auto& scene : m_ActiveScenes)
        {
            if (entity.GetScene() != scene.get())
                continue;

            return scene->m_Registry.valid(static_cast<entt::entity>(entity));
        }

        return false;
    }

    void SceneManager::DestroyEntity(GUID entity_guid) const
    {
        Entity entity = GetEntity(entity_guid);
        if (!entity)
            return;

        entity.Destroy();
    }

    void SceneManager::DestroyEntity(const std::string &entity_name) const
    {
        Entity entity = GetEntity(entity_name);
        if (!entity)
            return;

        entity.Destroy();
    }

    void SceneManager::OnStart()
    {
        SceneRenderer::Init();
    }
    
    void SceneManager::OnStop()
    {
        SceneRenderer::Shutdown();
    }

    void SceneManager::OnStartRuntime()
    {
    }
    
    void SceneManager::OnStopRuntime()
    {
    }

    void SceneManager::OnStartSimulation()
    {
    }

    void SceneManager::OnStopSimulation()
    {
    }

    void SceneManager::OnStartPhysics()
    {
    }

    void SceneManager::OnStopPhysics()
    {
    }

    void SceneManager::OnUpdate()
    {
        BC_PROFILE_SCOPE("SceneManager::OnUpdate: On Update Loop");

        for(auto scene : m_ActiveScenes)
        {
            scene->OnUpdate();
        }
    }

    void SceneManager::OnFixedUpdate()
    {
        BC_PROFILE_SCOPE_ACCUMULATIVE("SceneManager::OnFixedUpdate: On Fixed Update Loop");

        for(auto scene : m_ActiveScenes)
        {
            scene->OnFixedUpdate();
        }
    }

    void SceneManager::OnLateUpdate()
    {
        BC_PROFILE_SCOPE("SceneManager::OnLateUpdate: On Late Update Loop");

        for(auto scene : m_ActiveScenes)
        {
            scene->OnLateUpdate();
        }
    }
}