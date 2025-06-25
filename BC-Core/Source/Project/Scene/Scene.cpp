#include "BC_PCH.h"

// Core Headers
#include "Scene.h"
#include "Entity.h"

#include "Util/Hash.h"
#include "Util/FileUtil.h"

// C++ Standard Library Headers

// External Vendor Library Headers
#include <yaml-cpp/yaml.h>

namespace BC
{

#pragma region Scene Constructors

    Scene::Scene() :
        m_SceneID(GUID()),
        m_Octree(std::make_shared<OctreeBounds<Entity>>())
    {
    }

    Scene::Scene(const std::filesystem::path& scene_file_path) :
        m_SceneID(Util::HashStringInsensitive(Util::NormaliseFilePathToString(scene_file_path))),
        m_SceneName(scene_file_path.stem().string()),
        m_SceneFilePath(scene_file_path),
        m_Octree(std::make_shared<OctreeBounds<Entity>>())
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

    Entity Scene::CreateEntity(const std::string &name, GUID parent_guid)
    {
        return CreateEntity(GUID(), name, parent_guid);
    }

    Entity Scene::CreateEntity(GUID entity_guid, const std::string &name, GUID parent_guid)
    {
        // Obtain lock
		std::unique_lock<std::mutex> scene_octree_lock(m_Octree->GetOctreeMutex());

		while (m_EntityMap.find(entity_guid) != m_EntityMap.end()) 
		{
			entity_guid = GUID();
		}

		Entity entity = { m_Registry.create(), this };

        // 1. Transform Component
		entity.AddComponent<TransformComponent>();

        // 2. Meta Component
        auto& meta_component = entity.AddComponent<MetaComponent>();
        meta_component.SetEntityGUID(entity_guid);

        if (parent_guid != NULL_GUID)
            meta_component.AttachParent(parent_guid);
        meta_component.SetUniqueName(name);

		m_EntityMap.emplace(entity_guid, entity);

        MarkHierarchyDirty();

        return entity;
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

    void Scene::DuplicateEntity(const Entity &entity)
    {


        MarkHierarchyDirty();
    }

    void Scene::DestroyEntity(const Entity &entity)
    {
        // Obtain lock
		std::unique_lock<std::mutex> scene_octree_lock(m_Octree->GetOctreeMutex());

        DestroyEntityHelper(entity, m_EntityMap);

        MarkHierarchyDirty();
    }
    
#pragma endregion

#pragma region Functionality

    void Scene::OnUpdate()
    {
        m_HierarchyChangedThisFrame.store(false);
        // TODO: Implement
    }
    
    void Scene::OnFixedUpdate()
    {
        // TODO: Implement
    }
    
    void Scene::OnLateUpdate()
    {
        // TODO: Implement
    }

#pragma endregion

#pragma region Serialiation

    namespace Util
    {
        template<typename... Component>
        static void SerialiseComponent(YAML::Emitter& out, const Entity& entity)
        {
            ([&]()
                {
                    if (entity.HasComponent<Component>())
                        entity.GetComponent<Component>().SceneSerialise(out);
                }(), ...);
        }

        template<typename... Component>
        static void SerialiseComponent(ComponentGroup<Component...>, YAML::Emitter& out, const Entity& entity)
        {
            SerialiseComponent<Component...>(out, entity);
        }

        void SerialiseEntity(YAML::Emitter& out, const Entity& entity)
        {
            out << YAML::BeginMap;
            out << YAML::Key << "Entity ID" << YAML::Value << static_cast<uint64_t>(entity.GetGUID());
            out << YAML::Key << "Entity Name" << YAML::Value << entity.GetName();
            SerialiseComponent(AllComponents{}, out, entity);
            out << YAML::EndMap;
        }
        
        template<typename... Component>
        static void DeserialiseComponent(const YAML::Node& data, Entity& entity)
        {
            ([&]()
                {
                    auto component_data = data[Util::ComponentTypeToString(Entity::GetBlankComponent<Component>().GetType()).c_str()];
                    if (component_data)
                    {
                        if (!entity.HasComponent<Component>())
                            entity.AddComponent<Component>();

                        if (!entity.GetComponent<Component>().SceneDeserialise(component_data))
                            BC_CORE_WARN("Util::DeserialiseComponent: Deserialisation of {} not complete.", Util::ComponentTypeToString(Entity::GetBlankComponent<Component>().GetType()));
                    }
                }(), ...);
        }

        template<typename... Component>
        static void DeserialiseComponent(ComponentGroup<Component...>, const YAML::Node& data, Entity& entity)
        {
            DeserialiseComponent<Component...>(data, entity);
        }

        void DeserialiseEntity(const YAML::Node& data, Entity& entity)
        {
            DeserialiseComponent(AllComponents{}, data, entity);
        }
    }

    void Scene::Serialise()
    {
        std::filesystem::path out_path = Application::GetProject()->GetDirectory() / "Scenes" / m_SceneFilePath;

        if (out_path.extension() != ".scene")
            out_path.replace_extension(".scene");
        
        YAML::Emitter out;
        out << YAML::BeginMap;
        {
		    out << YAML::Key << "Scene ID" << YAML::Value << static_cast<uint64_t>(m_SceneID);
		    out << YAML::Key << "Scene Name" << YAML::Value << m_SceneName;

		    out << YAML::Key << "Entities" << YAML::Value;
            out << YAML::BeginSeq;
            {
                auto view = m_Registry.view<entt::entity>();
                for (auto entity_handle : view) 
                {
                    Entity entity = { entity_handle, this };
                    if (!entity)
                        continue;

                    Util::SerialiseEntity(out, entity);
                }
            }
            out << YAML::EndSeq;

        }
        out << YAML::EndMap;

        std::filesystem::create_directories(out_path.parent_path());

		std::ofstream fout(out_path);
        BC_ASSERT(fout.is_open(), "Scene::Serialise: Could Not Open File.");
		fout << out.c_str();
    }

    void Scene::Deserialise(const std::filesystem::path& scene_file_path)
    {
        if (scene_file_path.extension() != ".scene" || !std::filesystem::exists(scene_file_path))
            return;

        YAML::Node data = YAML::LoadFile(scene_file_path.string());

        if (data["Scene ID"])
            m_SceneID = data["Scene ID"].as<uint64_t>();
        
        if (data["Scene Name"])
            m_SceneName = data["Scene Name"].as<std::string>();

        if (!data["Entities"])
            return;

        for (const auto& entity_data : data["Entities"])
        {
            GUID entity_guid = entity_data["Entity ID"].as<uint64_t>();
            std::string entity_name = entity_data["Entity Name"].as<std::string>();

            Entity entity = CreateEntity(entity_guid, entity_name);

            Util::DeserialiseEntity(entity_data, entity);
        }

        auto transform_update = GetAllEntitiesWith<TransformComponent>();
        for (const auto& entity_handle : transform_update)
        {
            transform_update.get<TransformComponent>(entity_handle).SetPosition(transform_update.get<TransformComponent>(entity_handle).GetLocalPosition());
        }

		// Generate Octree for Scene
		{
			OctreeBoundsConfig octree_config = {};

			std::vector<OctreeBounds<Entity>::OctreeData> data_sources;

			auto static_mesh_view = GetAllEntitiesWith<MeshRendererComponent>();
			for (const auto& entity_handle : static_mesh_view) 
            {
				auto& mesh_component = static_mesh_view.get<MeshRendererComponent>(entity_handle);

				// Ensure the AABB is up to date
				mesh_component.UpdateTransformedAABB();

				const auto& aabb = mesh_component.GetTransformedAABB();

				if (!mesh_component.GetEntity()) 
                {
					BC_CORE_ERROR("Scene::CopyScene: Cannot Insert Entity to Octree - Current Entity Is Invalid.");
					continue;
				}

				data_sources.push_back(std::make_shared<OctreeDataSource<Entity>>(mesh_component.GetEntity(), aabb));
			}

			auto skinned_mesh_view = GetAllEntitiesWith<SkinnedMeshRendererComponent>();
			for (const auto& entity_handle : skinned_mesh_view) 
            {
				auto& mesh_component = skinned_mesh_view.get<SkinnedMeshRendererComponent>(entity_handle);

				// Ensure the AABB is up to date
				mesh_component.UpdateTransformedAABB();

				const auto& aabb = mesh_component.GetTransformedAABB();

				if (!mesh_component.GetEntity()) 
                {
					BC_CORE_ERROR("Scene::CopyScene: Cannot Insert Entity to Octree - Current Entity Is Invalid.");
					continue;
				}

				data_sources.push_back(std::make_shared<OctreeDataSource<Entity>>(mesh_component.GetEntity(), aabb));
			}

			octree_config.Looseness = 1.25f;
			octree_config.PreferredDataSourceLimit = 8;

			// Create the octree and insert data sources
			m_Octree = std::make_shared<OctreeBounds<Entity>>(octree_config, data_sources);
		}
        
        MarkHierarchyDirty();
    }

#pragma endregion

#pragma region Static

    std::shared_ptr<Scene> Scene::CreateDefaultScene(const std::filesystem::path &scene_file_path)
    {
        std::shared_ptr<Scene> default_scene = std::make_shared<Scene>(scene_file_path);

        Entity camera_entity    = default_scene->CreateEntity("Main Camera");
        Entity light_entity     = default_scene->CreateEntity("Directional Light");
        Entity cube_entity      = default_scene->CreateEntity("Cube");

        camera_entity.AddComponent<CameraComponent>();
        camera_entity.GetTransform().SetPosition({ 0.0f, 5.0f, 10.0f });
        camera_entity.GetTransform().LookAt({ 0.0f, 0.0f, 0.0f });
        
        light_entity.AddComponent<DirectionalLightComponent>();
        light_entity.GetTransform().SetOrientationEuler({-45.0f, 45.0f, 0.0f});

        cube_entity.AddComponent<MeshRendererComponent>();
        // TODO: Add Default MeshHandle for Cube from "Resources/Models/Cube.fbx"

        default_scene->m_SceneName = scene_file_path.stem().string();
        default_scene->m_SceneFilePath = scene_file_path;
        
        return default_scene;
    }

    std::shared_ptr<Scene> Scene::LoadScene(const std::filesystem::path& scene_file_path, const std::filesystem::path& project_directory)
    {        
        auto scene = std::make_shared<Scene>(scene_file_path);
        scene->Deserialise(project_directory.empty() ? scene_file_path : project_directory / "Scenes" / scene_file_path);
        return scene;
    }

    namespace Util
    {
        template<typename... Component>
        static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<GUID, entt::entity>& guid_to_entity_map, Scene* dest_scene)
        {
            ([&]()
                {
                    auto view = src.view<Component>();
                    for (auto source_entity_handle : view)
                    {
                        auto& source_component = src.get<Component>(source_entity_handle);

                        Entity source_entity = source_component.GetEntity();
                        if (!source_entity)
                            continue;

                        entt::entity dest_entity_handle = guid_to_entity_map.at(source_entity.GetGUID());
                        Entity dest_entity = { dest_entity_handle, dest_scene };

                        // Temporarily Set Source Entity to Destination Entity for Component Copying
                        source_component.SetEntity(dest_entity);

                        // Call Copy Constructor of Component
                        auto& dest_component = dst.emplace_or_replace<Component>(dest_entity_handle, source_component);

                        // Revert Source Component Entity Reference to Correct Reference
                        source_component.SetEntity(source_entity);

                        // Ensure Destination Component Has Correct Entity
                        dest_component.SetEntity(dest_entity);
                    }
                }(), ...);
        }

        template<typename... Component>
        static void CopyComponent(ComponentGroup<Component...>, entt::registry& dst, entt::registry& src, const std::unordered_map<GUID, entt::entity>& guid_to_entity_map, Scene* dest_scene)
        {
            CopyComponent<Component...>(dst, src, guid_to_entity_map, dest_scene);
        }

        void CopyRegistry(std::shared_ptr<Scene> source_scene, std::shared_ptr<Scene> dest_scene)
        {
            auto source_registry = source_scene->GetRegistry();
            auto dest_registry = dest_scene->GetRegistry();

            std::unordered_map<GUID, entt::entity> guid_to_entity_map;

            // Create entities in new scene
            auto view = source_registry->view<MetaComponent>();
            for (const auto& entity_handle : view)
            {
                auto& source_meta_component = view.get<MetaComponent>(entity_handle);
                Entity new_entity = dest_scene->CreateEntity(source_meta_component.GetEntityGUID(), source_meta_component.GetName());

                auto& dest_meta_component = new_entity.GetComponent<MetaComponent>();
                dest_meta_component = source_meta_component;

                guid_to_entity_map[new_entity.GetGUID()] = (entt::entity)new_entity;
            }

            CopyComponent(AllComponents{}, *dest_registry, *source_registry, guid_to_entity_map, dest_scene.get());
        }
    }

    std::shared_ptr<Scene> Scene::CopyScene(std::shared_ptr<Scene> source_scene)
    {
        std::shared_ptr<Scene> dest_scene = std::make_shared<Scene>();

        dest_scene->m_EntityMap = source_scene->m_EntityMap;
        
        dest_scene->m_SceneID = source_scene->m_SceneID;
        dest_scene->m_SceneName = source_scene->m_SceneName;
        dest_scene->m_SceneFilePath = source_scene->m_SceneFilePath;

        Util::CopyRegistry(source_scene, dest_scene);

		// Generate Octree for New Scene
		{
			OctreeBoundsConfig octree_config = source_scene->GetOctree()->GetConfig();

			std::vector<OctreeBounds<Entity>::OctreeData> data_sources;

			auto static_mesh_view = dest_scene->GetAllEntitiesWith<MeshRendererComponent>();
			for (const auto& entity_handle : static_mesh_view) 
            {
				auto& mesh_component = static_mesh_view.get<MeshRendererComponent>(entity_handle);

				// Ensure the AABB is up to date
				mesh_component.UpdateTransformedAABB();

				const auto& aabb = mesh_component.GetTransformedAABB();

				if (!mesh_component.GetEntity()) 
                {
					BC_CORE_ERROR("Scene::CopyScene: Cannot Insert Entity to Octree - Current Entity Is Invalid.");
					continue;
				}

				data_sources.push_back(std::make_shared<OctreeDataSource<Entity>>(mesh_component.GetEntity(), aabb));
			}

			auto skinned_mesh_view = dest_scene->GetAllEntitiesWith<SkinnedMeshRendererComponent>();
			for (const auto& entity_handle : skinned_mesh_view) 
            {
				auto& mesh_component = skinned_mesh_view.get<SkinnedMeshRendererComponent>(entity_handle);

				// Ensure the AABB is up to date
				mesh_component.UpdateTransformedAABB();

				const auto& aabb = mesh_component.GetTransformedAABB();

				if (!mesh_component.GetEntity()) 
                {
					BC_CORE_ERROR("Scene::CopyScene: Cannot Insert Entity to Octree - Current Entity Is Invalid.");
					continue;
				}

				data_sources.push_back(std::make_shared<OctreeDataSource<Entity>>(mesh_component.GetEntity(), aabb));
			}

			octree_config.Looseness = 1.25f;
			octree_config.PreferredDataSourceLimit = 8;

			// Create the octree and insert data sources
			dest_scene->m_Octree = std::make_shared<OctreeBounds<Entity>>(octree_config, data_sources);
		}

        return dest_scene;
    }

#pragma endregion

}