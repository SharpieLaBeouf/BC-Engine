#include "BC_PCH.h"
#include "SceneManager.h"

// Core Headers
#include "Asset/AssetManagerAPI.h"
#include "Graphics/Renderer/SceneRenderer.h"

#include "Util/Hash.h"
#include "Util/FileUtil.h"

// C++ Standard Library Headers

// External Vendor Library Headers
#include<physx/PxPhysicsAPI.h>

#include <yaml-cpp/yaml.h>

namespace BC
{
    
	static PxFilterFlags CustomFilterShader(
		PxFilterObjectAttributes attributes0, PxFilterData filterData0,
		PxFilterObjectAttributes attributes1, PxFilterData filterData1,
		PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
	{
		pairFlags = PxPairFlag::eCONTACT_DEFAULT | PxPairFlag::eNOTIFY_TOUCH_FOUND | PxPairFlag::eNOTIFY_TOUCH_PERSISTS | PxPairFlag::eNOTIFY_TOUCH_LOST;
		return PxFilterFlag::eDEFAULT;
	}

    SceneManager::SceneManager()
    {
    }

    SceneManager::~SceneManager()
    {
    }

    Entity SceneManager::GetEntity(GUID entity_guid) const
    {
        if (entity_guid == NULL_GUID)
            return Entity{};

        for(auto [scene_id, scene] : m_SceneInstances)
        {
            if (scene->m_EntityMap.contains(entity_guid))
                return Entity{ scene->m_EntityMap.at(entity_guid), scene.get() };
        }
        return Entity{};
    }

    Entity SceneManager::GetEntity(const std::string &entity_name) const
    {
        for(auto [scene_id, scene] : m_SceneInstances)
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
        for(auto [scene_id, scene] : m_SceneInstances)
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
    }
    
    void SceneManager::OnStop()
    {
		if (m_IsRunning)
			OnStopRuntime();

		if (m_IsSimulating)
			OnStopSimulation();

		m_IsRunning = false;
		m_IsSimulating = false;
    }

    void SceneManager::OnStartRuntime()
    {
		m_IsRunning = true;

		OnStartPhysics();

        auto view = GetAllEntitiesWithComponent<MetaComponent>();
        for (const auto& entity : view)
        {
            if (!entity)
                continue;

            // TODO: Instantiate Script Instances
            
            if (entity.HasComponent<AnimatorComponent>())
            {
                entity.GetComponent<AnimatorComponent>().Init();
            }
        }
    }
    
    void SceneManager::OnStopRuntime()
    {
		m_IsRunning = false;
		m_IsSimulating = false;

		//ScriptManager::Get()->RemoveAllScriptInstances();
		AssetManager::ClearRuntimeAssets();
		OnStopPhysics();

        auto view = GetAllEntitiesWithComponent<AnimatorComponent>();
        for (const auto& entity : view)
        {
            if (!entity)
                continue;

            entity.GetComponent<AnimatorComponent>().Shutdown();
        }
    }

    void SceneManager::OnStartSimulation()
    {
		m_IsSimulating = true;

		OnStartPhysics();

        auto view = GetAllEntitiesWithComponent<AnimatorComponent>();
        for (const auto& entity : view)
        {
            if (!entity)
                continue;

            entity.GetComponent<AnimatorComponent>().Init();
        }
    }

    void SceneManager::OnStopSimulation()
    {
		m_IsRunning = false;
		m_IsSimulating = false;

		OnStopPhysics();

        auto view = GetAllEntitiesWithComponent<AnimatorComponent>();
        for (const auto& entity : view)
        {
            if (!entity)
                continue;

            entity.GetComponent<AnimatorComponent>().Shutdown();
        }
    }

    void SceneManager::OnStartPhysics()
    {
        m_PhysicsSystem = std::make_unique<PhysicsSystem>();
        m_PhysicsSystem->Init();

        std::unordered_set<Entity> physics_entities = {};

        for (auto&& e : GetAllEntitiesWithComponent<RigidbodyComponent>()) physics_entities.insert(e);
        for (auto&& e : GetAllEntitiesWithComponent<PlaneCollider>()) physics_entities.insert(e);
        for (auto&& e : GetAllEntitiesWithComponent<BoxColliderComponent>()) physics_entities.insert(e);
        for (auto&& e : GetAllEntitiesWithComponent<SphereColliderComponent>()) physics_entities.insert(e);
        for (auto&& e : GetAllEntitiesWithComponent<CapsuleColliderComponent>()) physics_entities.insert(e);
        for (auto&& e : GetAllEntitiesWithComponent<ConvexMeshColliderComponent>()) physics_entities.insert(e);
        for (auto&& e : GetAllEntitiesWithComponent<HeightFieldColliderComponent>()) physics_entities.insert(e);
        for (auto&& e : GetAllEntitiesWithComponent<TriangleMeshColliderComponent>()) physics_entities.insert(e);

		for (const auto& entity : physics_entities) 
        {
			if (entity.HasComponent<RigidbodyComponent>())
				entity.GetComponent<RigidbodyComponent>().Init();

			if (entity.HasComponent<PlaneCollider>())
				entity.GetComponent<PlaneCollider>().Init();

			if (entity.HasComponent<BoxColliderComponent>())
				entity.GetComponent<BoxColliderComponent>().Init();

			if (entity.HasComponent<SphereColliderComponent>())
				entity.GetComponent<SphereColliderComponent>().Init();

			if (entity.HasComponent<CapsuleColliderComponent>())
				entity.GetComponent<CapsuleColliderComponent>().Init();

			if (entity.HasComponent<ConvexMeshColliderComponent>())
				entity.GetComponent<ConvexMeshColliderComponent>().Init();

			if (entity.HasComponent<HeightFieldColliderComponent>())
				entity.GetComponent<HeightFieldColliderComponent>().Init();

			if (entity.HasComponent<TriangleMeshColliderComponent>())
				entity.GetComponent<TriangleMeshColliderComponent>().Init();
		}
    }

    void SceneManager::OnStopPhysics()
    {
        std::unordered_set<Entity> physics_entities = {};

        for (auto&& e : GetAllEntitiesWithComponent<RigidbodyComponent>())              physics_entities.insert(e);
        for (auto&& e : GetAllEntitiesWithComponent<PlaneCollider>())                   physics_entities.insert(e);
        for (auto&& e : GetAllEntitiesWithComponent<BoxColliderComponent>())            physics_entities.insert(e);
        for (auto&& e : GetAllEntitiesWithComponent<SphereColliderComponent>())         physics_entities.insert(e);
        for (auto&& e : GetAllEntitiesWithComponent<CapsuleColliderComponent>())        physics_entities.insert(e);
        for (auto&& e : GetAllEntitiesWithComponent<ConvexMeshColliderComponent>())     physics_entities.insert(e);
        for (auto&& e : GetAllEntitiesWithComponent<HeightFieldColliderComponent>())    physics_entities.insert(e);
        for (auto&& e : GetAllEntitiesWithComponent<TriangleMeshColliderComponent>())   physics_entities.insert(e);

		for (const auto& entity : physics_entities) 
        {
			if (entity.HasComponent<RigidbodyComponent>())
				entity.GetComponent<RigidbodyComponent>().Shutdown();

			if (entity.HasComponent<PlaneCollider>())
				entity.GetComponent<PlaneCollider>().Shutdown();

			if (entity.HasComponent<BoxColliderComponent>())
				entity.GetComponent<BoxColliderComponent>().Shutdown();

			if (entity.HasComponent<SphereColliderComponent>())
				entity.GetComponent<SphereColliderComponent>().Shutdown();

			if (entity.HasComponent<CapsuleColliderComponent>())
				entity.GetComponent<CapsuleColliderComponent>().Shutdown();

			if (entity.HasComponent<ConvexMeshColliderComponent>())
				entity.GetComponent<ConvexMeshColliderComponent>().Shutdown();

			if (entity.HasComponent<HeightFieldColliderComponent>())
				entity.GetComponent<HeightFieldColliderComponent>().Shutdown();

			if (entity.HasComponent<TriangleMeshColliderComponent>())
				entity.GetComponent<TriangleMeshColliderComponent>().Shutdown(); 
		}

        m_PhysicsSystem->Shutdown();
        m_PhysicsSystem.reset();
        m_PhysicsSystem = nullptr;
    }

    void SceneManager::OnUpdate()
    {
        BC_PROFILE_SCOPE("SceneManager::OnUpdate: On Update Loop");

        for(auto [scene_id, scene] : m_SceneInstances)
        {
            scene->OnUpdate();
        }
    }

    void SceneManager::OnFixedUpdate()
    {
        BC_PROFILE_SCOPE_ACCUMULATIVE("SceneManager::OnFixedUpdate: On Fixed Update Loop");

        for(auto [scene_id, scene] : m_SceneInstances)
        {
            scene->OnFixedUpdate();
        }

        if (m_PhysicsSystem)
            m_PhysicsSystem->OnUpdate();
    }

    void SceneManager::OnLateUpdate()
    {
        BC_PROFILE_SCOPE("SceneManager::OnLateUpdate: On Late Update Loop");

        for(auto [scene_id, scene] : m_SceneInstances)
        {
            scene->OnLateUpdate();
        }
    }

    void SceneManager::LoadScene(const std::string &scene_name, bool additive)
    {
        for (const auto& [scene_id, scene_path] : m_SceneFilePaths)
        {
            if (scene_name == scene_path.stem().string())
                LoadScene(scene_id, additive);
        }
        BC_CORE_WARN("SceneManager::LoadScene: Could Not Find Scene Name: '{}'", scene_name);
    }

    void SceneManager::LoadSceneAsync(const std::string &scene_name, bool additive)
    {
        for (const auto& [scene_id, scene_path] : m_SceneFilePaths)
        {
            if (scene_name == scene_path.stem().string())
                LoadSceneAsync(scene_id, additive);
        }
        BC_CORE_WARN("SceneManager::LoadSceneAsync: Could Not Find Scene Name: '{}'", scene_name);
    }

    void SceneManager::LoadScene(GUID scene_guid, bool additive, const std::filesystem::path &project_directory)
    {
        // Don't load if doesn't exist in templates OR if already loaded
        if (!m_SceneFilePaths.contains(scene_guid) || m_SceneInstances.contains(scene_guid))
            return;

        BC_ASSERT
        (
            Util::HashStringInsensitive(Util::NormaliseFilePathToString(m_SceneFilePaths[scene_guid])) == scene_guid, 
            "SceneManager::LoadScene: Scene Manager ID Mismatch With Runtime Hash of Scene File Path."
        );

        m_SceneInstances[scene_guid] = Scene::LoadScene(m_SceneFilePaths[scene_guid], project_directory);
        m_SceneInstances[scene_guid]->m_SceneID = scene_guid;
    }
    
    void SceneManager::LoadSceneAsync(GUID scene_guid, bool additive, const std::filesystem::path& project_directory)
    {
        if (!m_SceneFilePaths.contains(scene_guid))
            return;
        
        BC_ASSERT
        (
            Util::HashStringInsensitive(Util::NormaliseFilePathToString(m_SceneFilePaths[scene_guid])) == scene_guid, 
            "SceneManager::LoadSceneAsync: SceneManager Cached ID Mismatch With Runtime Hash of Scene File Path."
        );
        
        Application::GetJobSystem()->SubmitJob
        (
            "SceneManager::LoadSceneAsync",
            [&]()
            {
                m_SceneInstances[scene_guid] = Scene::LoadScene(m_SceneFilePaths[scene_guid], project_directory);
                m_SceneInstances[scene_guid]->m_SceneID = scene_guid;
            },
            nullptr,
            JobPriority::Medium,
            false
        );
    }

    void SceneManager::AddSceneTemplate(std::shared_ptr<Scene> scene)
    {
        if (scene->m_SceneFilePath.empty())
            return;

        if (scene->m_SceneFilePath.extension() != ".scene")
            scene->m_SceneFilePath.replace_extension(".scene");

        m_SceneInstances[scene->m_SceneID] = scene;
        m_SceneFilePaths[scene->m_SceneID] = scene->m_SceneFilePath;
    }

    void SceneManager::AddSceneTemplate(GUID scene_id, const std::filesystem::path& scene_file_path)
    {
        if (!std::filesystem::exists(scene_file_path))
            return;

        m_SceneFilePaths[scene_id] = scene_file_path;
    }

    void SceneManager::SetActiveScene(GUID scene_id)
    {
        if (!m_SceneInstances.contains(scene_id))
            return;
        
        m_ActiveScene = scene_id;
    }

    void SceneManager::SetActiveScene(const std::string& scene_name)
    {
        for (const auto& [scene_id, scene] : m_SceneInstances)
        {
            if (scene_name == scene->GetName())
            {
                m_ActiveScene = scene_id;
                return;
            }
        }
    }

    std::shared_ptr<Scene> SceneManager::GetActiveScene()
    {
        // Populate scene file paths from loaded instances if needed
        if (m_SceneFilePaths.empty())
        {
            if (!m_SceneInstances.empty())
            {
                bool entry_valid = false;

                for (const auto& [scene_id, scene] : m_SceneInstances)
                {
                    m_SceneFilePaths[scene_id] = scene->m_SceneFilePath;
                    if (scene_id == m_EntryScene)
                        entry_valid = true;
                }

                if (!entry_valid)
                    m_EntryScene = m_SceneInstances.begin()->first;
            }
            else
            {
                auto default_scene = Scene::CreateDefaultScene("Untitled Scene.scene");

                m_EntryScene = default_scene->m_SceneID;
                m_ActiveScene = m_EntryScene;
                m_SceneFilePaths[m_EntryScene] = default_scene->m_SceneFilePath;
                m_SceneInstances[m_EntryScene] = std::move(default_scene);
            }
        }

        // Ensure Entry Scene is valid
        if (m_EntryScene == NULL_GUID && !m_SceneFilePaths.empty())
            m_EntryScene = m_SceneFilePaths.begin()->first;

        // Ensure Active Scene is valid
        if (m_ActiveScene == NULL_GUID)
            m_ActiveScene = m_EntryScene;

        // Load the active scene if it's not loaded yet
        if (!m_SceneInstances.contains(m_ActiveScene) && m_SceneFilePaths.contains(m_ActiveScene))
        {
            auto loaded_scene = Scene::LoadScene(m_SceneFilePaths[m_ActiveScene]);
            if (loaded_scene)
            {
                loaded_scene->m_SceneID = m_ActiveScene;
                m_SceneInstances[m_ActiveScene] = std::move(loaded_scene);
            }
        }

        auto it = m_SceneInstances.find(m_ActiveScene);
        BC_ASSERT(it != m_SceneInstances.end(), "SceneManager::GetActiveScene: Could not retrieve active scene.");
        return (it != m_SceneInstances.end()) ? it->second : nullptr;
    }

    void SceneManager::Serialise(YAML::Emitter &out)
    {
        out << YAML::Key << "Scene Manager";
        out << YAML::BeginMap;
        {
            out << YAML::Key << "Entry Scene" << YAML::Value << static_cast<uint64_t>(m_EntryScene);

            out << YAML::Key << "Scenes";
            out << YAML::BeginSeq;
            for (const auto& [scene_id, scene_file_path] : m_SceneFilePaths)
            {
                out << YAML::BeginMap;
                {
                    out << YAML::Key << "ID" << YAML::Value << static_cast<uint64_t>(scene_id);
                    out << YAML::Key << "Name" << YAML::Value << scene_file_path.stem().string();
                    out << YAML::Key << "Relative Path" << YAML::Value << scene_file_path.string();
                }
                out << YAML::EndMap;
            }
            out << YAML::EndSeq;
        }
        out << YAML::EndMap;
    }

    void SceneManager::Deserialise(const YAML::Node& data, const std::filesystem::path& project_directory)
    {
        if (data["Entry Scene"])
            m_EntryScene = data["Entry Scene"].as<uint64_t>();

        m_ActiveScene = m_EntryScene;
        
        if (!data["Scenes"])
            return;
        
        for (const auto& scene_data : data["Scenes"])
        {
            if (!scene_data["ID"] || !scene_data["Relative Path"])
                continue;

            m_SceneFilePaths[scene_data["ID"].as<uint64_t>()] = scene_data["Relative Path"].as<std::string>();
        }
    }

    void SceneManager::SaveAllScenes()
    {
        for (auto& [scene_id, scene] : m_SceneInstances)
        {            
            scene->Serialise();
        }
    }
}