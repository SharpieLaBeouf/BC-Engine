#include "BC_PCH.h"
#include "Component Base.h"

#include "Project/Scene/Entity.h"

namespace BC
{

#pragma region Component Base Functions

    GUID ComponentBase::GetGUID() const  { return m_EntityID; }
    void ComponentBase::SetGUID(GUID entity_guid) { m_EntityID = entity_guid; }

    Entity ComponentBase::GetEntity() const
    {
        return Application::GetProject()->GetSceneManager()->GetEntity(m_EntityID);
    }

    template <typename T>
    T& ComponentBase::GetComponent() const
    {
        auto entity = Application::GetProject()->GetSceneManager()->GetEntity(m_EntityID);
        if (!entity)
        {
            return Entity::GetBlankComponent<T>();
        }
        return entity.GetComponent<T>();
    }

    template <typename T>
    T& ComponentBase::GetComponentInParent() const
    {
        auto entity = Application::GetProject()->GetSceneManager()->GetEntity(m_EntityID);
        if (!entity)
        {
            return Entity::GetBlankComponent<T>();
        }
        return entity.GetComponentInParent<T>();
    }

    template <typename T>
    T& ComponentBase::GetComponentInChild() const
    {
        auto entity = Application::GetProject()->GetSceneManager()->GetEntity(m_EntityID);
        if (!entity)
        {
            return Entity::GetBlankComponent<T>();
        }
        return entity.GetComponentInChild<T>();
    }

    template <typename T>
    std::vector<Entity> ComponentBase::GetComponentsInParents() const
    {
        auto entity = Application::GetProject()->GetSceneManager()->GetEntity(m_EntityID);
        if (!entity)
        {
            return std::vector<Entity>();
        }
        return entity.GetComponentsInParents<T>();
    }

    template <typename T>
    std::vector<Entity> ComponentBase::GetComponentsInChildren() const
    {
        auto entity = Application::GetProject()->GetSceneManager()->GetEntity(m_EntityID);
        if (!entity)
        {
            return std::vector<Entity>();
        }
        return entity.GetComponentsInChildren<T>();
    }

#pragma endregion

#pragma region Component Base Function Template Specialisations

	// Specialisations for GetComponent
	template TransformComponent&			    ComponentBase::GetComponent<TransformComponent>() const;
	template MetaComponent&			            ComponentBase::GetComponent<MetaComponent>() const;
	template CameraComponent&				    ComponentBase::GetComponent<CameraComponent>() const;
	template AudioListenerComponent&		    ComponentBase::GetComponent<AudioListenerComponent>() const;
	template AudioEmitterComponent&			    ComponentBase::GetComponent<AudioEmitterComponent>() const;
	template LODMeshComponent&				    ComponentBase::GetComponent<LODMeshComponent>() const;
	template MeshRendererComponent&			    ComponentBase::GetComponent<MeshRendererComponent>() const;
	template SkinnedMeshRendererComponent&	    ComponentBase::GetComponent<SkinnedMeshRendererComponent>() const;
	template SimpleAnimationComponent&		    ComponentBase::GetComponent<SimpleAnimationComponent>() const;
	template AnimatorComponent&				    ComponentBase::GetComponent<AnimatorComponent>() const;
	template SphereLightComponent&			    ComponentBase::GetComponent<SphereLightComponent>() const;
	template ConeLightComponent&			    ComponentBase::GetComponent<ConeLightComponent>() const;
	template DirectionalLightComponent&		    ComponentBase::GetComponent<DirectionalLightComponent>() const;
	template RigidbodyComponent&			    ComponentBase::GetComponent<RigidbodyComponent>() const;
	template PlaneCollider&			            ComponentBase::GetComponent<PlaneCollider>() const;
	template BoxColliderComponent&			    ComponentBase::GetComponent<BoxColliderComponent>() const;
	template SphereColliderComponent&		    ComponentBase::GetComponent<SphereColliderComponent>() const;
	template CapsuleColliderComponent&			ComponentBase::GetComponent<CapsuleColliderComponent>() const;
	template ConvexMeshColliderComponent&		ComponentBase::GetComponent<ConvexMeshColliderComponent>() const;
	template HeightFieldColliderComponent&		ComponentBase::GetComponent<HeightFieldColliderComponent>() const;
	template TriangleMeshColliderComponent&		ComponentBase::GetComponent<TriangleMeshColliderComponent>() const;
    
	// Specialisations for GetComponentInParent
	template TransformComponent&			    ComponentBase::GetComponentInParent<TransformComponent>() const;
	template MetaComponent&			            ComponentBase::GetComponentInParent<MetaComponent>() const;
	template CameraComponent&				    ComponentBase::GetComponentInParent<CameraComponent>() const;
	template AudioListenerComponent&		    ComponentBase::GetComponentInParent<AudioListenerComponent>() const;
	template AudioEmitterComponent&			    ComponentBase::GetComponentInParent<AudioEmitterComponent>() const;
	template LODMeshComponent&				    ComponentBase::GetComponentInParent<LODMeshComponent>() const;
	template MeshRendererComponent&			    ComponentBase::GetComponentInParent<MeshRendererComponent>() const;
	template SkinnedMeshRendererComponent&	    ComponentBase::GetComponentInParent<SkinnedMeshRendererComponent>() const;
	template SimpleAnimationComponent&		    ComponentBase::GetComponentInParent<SimpleAnimationComponent>() const;
	template AnimatorComponent&				    ComponentBase::GetComponentInParent<AnimatorComponent>() const;
	template SphereLightComponent&			    ComponentBase::GetComponentInParent<SphereLightComponent>() const;
	template ConeLightComponent&			    ComponentBase::GetComponentInParent<ConeLightComponent>() const;
	template DirectionalLightComponent&		    ComponentBase::GetComponentInParent<DirectionalLightComponent>() const;
	template RigidbodyComponent&			    ComponentBase::GetComponentInParent<RigidbodyComponent>() const;
	template PlaneCollider&			            ComponentBase::GetComponentInParent<PlaneCollider>() const;
	template BoxColliderComponent&			    ComponentBase::GetComponentInParent<BoxColliderComponent>() const;
	template SphereColliderComponent&		    ComponentBase::GetComponentInParent<SphereColliderComponent>() const;
	template CapsuleColliderComponent&			ComponentBase::GetComponentInParent<CapsuleColliderComponent>() const;
	template ConvexMeshColliderComponent&		ComponentBase::GetComponentInParent<ConvexMeshColliderComponent>() const;
	template HeightFieldColliderComponent&		ComponentBase::GetComponentInParent<HeightFieldColliderComponent>() const;
	template TriangleMeshColliderComponent&		ComponentBase::GetComponentInParent<TriangleMeshColliderComponent>() const;
    
	// Specialisations for GetComponentInChild
	template TransformComponent&			    ComponentBase::GetComponentInChild<TransformComponent>() const;
	template MetaComponent&			            ComponentBase::GetComponentInChild<MetaComponent>() const;
	template CameraComponent&				    ComponentBase::GetComponentInChild<CameraComponent>() const;
	template AudioListenerComponent&		    ComponentBase::GetComponentInChild<AudioListenerComponent>() const;
	template AudioEmitterComponent&			    ComponentBase::GetComponentInChild<AudioEmitterComponent>() const;
	template LODMeshComponent&				    ComponentBase::GetComponentInChild<LODMeshComponent>() const;
	template MeshRendererComponent&			    ComponentBase::GetComponentInChild<MeshRendererComponent>() const;
	template SkinnedMeshRendererComponent&	    ComponentBase::GetComponentInChild<SkinnedMeshRendererComponent>() const;
	template SimpleAnimationComponent&		    ComponentBase::GetComponentInChild<SimpleAnimationComponent>() const;
	template AnimatorComponent&				    ComponentBase::GetComponentInChild<AnimatorComponent>() const;
	template SphereLightComponent&			    ComponentBase::GetComponentInChild<SphereLightComponent>() const;
	template ConeLightComponent&			    ComponentBase::GetComponentInChild<ConeLightComponent>() const;
	template DirectionalLightComponent&		    ComponentBase::GetComponentInChild<DirectionalLightComponent>() const;
	template RigidbodyComponent&			    ComponentBase::GetComponentInChild<RigidbodyComponent>() const;
	template PlaneCollider&			            ComponentBase::GetComponentInChild<PlaneCollider>() const;
	template BoxColliderComponent&			    ComponentBase::GetComponentInChild<BoxColliderComponent>() const;
	template SphereColliderComponent&		    ComponentBase::GetComponentInChild<SphereColliderComponent>() const;
	template CapsuleColliderComponent&			ComponentBase::GetComponentInChild<CapsuleColliderComponent>() const;
	template ConvexMeshColliderComponent&		ComponentBase::GetComponentInChild<ConvexMeshColliderComponent>() const;
	template HeightFieldColliderComponent&		ComponentBase::GetComponentInChild<HeightFieldColliderComponent>() const;
	template TriangleMeshColliderComponent&		ComponentBase::GetComponentInChild<TriangleMeshColliderComponent>() const;
    
	// Specialisations for GetComponentsInParents
	template std::vector<Entity>    ComponentBase::GetComponentsInParents<TransformComponent>() const;
	template std::vector<Entity>	ComponentBase::GetComponentsInParents<MetaComponent>() const;
	template std::vector<Entity>	ComponentBase::GetComponentsInParents<CameraComponent>() const;
	template std::vector<Entity>	ComponentBase::GetComponentsInParents<AudioListenerComponent>() const;
	template std::vector<Entity>	ComponentBase::GetComponentsInParents<AudioEmitterComponent>() const;
	template std::vector<Entity>	ComponentBase::GetComponentsInParents<LODMeshComponent>() const;
	template std::vector<Entity>	ComponentBase::GetComponentsInParents<MeshRendererComponent>() const;
	template std::vector<Entity>	ComponentBase::GetComponentsInParents<SkinnedMeshRendererComponent>() const;
	template std::vector<Entity>	ComponentBase::GetComponentsInParents<SimpleAnimationComponent>() const;
	template std::vector<Entity>	ComponentBase::GetComponentsInParents<AnimatorComponent>() const;
	template std::vector<Entity>	ComponentBase::GetComponentsInParents<SphereLightComponent>() const;
	template std::vector<Entity>	ComponentBase::GetComponentsInParents<ConeLightComponent>() const;
	template std::vector<Entity>	ComponentBase::GetComponentsInParents<DirectionalLightComponent>() const;
	template std::vector<Entity>	ComponentBase::GetComponentsInParents<RigidbodyComponent>() const;
	template std::vector<Entity>	ComponentBase::GetComponentsInParents<PlaneCollider>() const;
	template std::vector<Entity>	ComponentBase::GetComponentsInParents<BoxColliderComponent>() const;
	template std::vector<Entity>	ComponentBase::GetComponentsInParents<SphereColliderComponent>() const;
	template std::vector<Entity>	ComponentBase::GetComponentsInParents<CapsuleColliderComponent>() const;
	template std::vector<Entity>	ComponentBase::GetComponentsInParents<ConvexMeshColliderComponent>() const;
	template std::vector<Entity>	ComponentBase::GetComponentsInParents<HeightFieldColliderComponent>() const;
	template std::vector<Entity>	ComponentBase::GetComponentsInParents<TriangleMeshColliderComponent>() const;
    
	// Specialisations for GetComponentsInChildren
	template std::vector<Entity>    ComponentBase::GetComponentsInChildren<TransformComponent>() const;
	template std::vector<Entity>	ComponentBase::GetComponentsInChildren<MetaComponent>() const;
	template std::vector<Entity>	ComponentBase::GetComponentsInChildren<CameraComponent>() const;
	template std::vector<Entity>	ComponentBase::GetComponentsInChildren<AudioListenerComponent>() const;
	template std::vector<Entity>	ComponentBase::GetComponentsInChildren<AudioEmitterComponent>() const;
	template std::vector<Entity>	ComponentBase::GetComponentsInChildren<LODMeshComponent>() const;
	template std::vector<Entity>	ComponentBase::GetComponentsInChildren<MeshRendererComponent>() const;
	template std::vector<Entity>	ComponentBase::GetComponentsInChildren<SkinnedMeshRendererComponent>() const;
	template std::vector<Entity>	ComponentBase::GetComponentsInChildren<SimpleAnimationComponent>() const;
	template std::vector<Entity>	ComponentBase::GetComponentsInChildren<AnimatorComponent>() const;
	template std::vector<Entity>	ComponentBase::GetComponentsInChildren<SphereLightComponent>() const;
	template std::vector<Entity>	ComponentBase::GetComponentsInChildren<ConeLightComponent>() const;
	template std::vector<Entity>	ComponentBase::GetComponentsInChildren<DirectionalLightComponent>() const;
	template std::vector<Entity>	ComponentBase::GetComponentsInChildren<RigidbodyComponent>() const;
	template std::vector<Entity>	ComponentBase::GetComponentsInChildren<PlaneCollider>() const;
	template std::vector<Entity>	ComponentBase::GetComponentsInChildren<BoxColliderComponent>() const;
	template std::vector<Entity>	ComponentBase::GetComponentsInChildren<SphereColliderComponent>() const;
	template std::vector<Entity>	ComponentBase::GetComponentsInChildren<CapsuleColliderComponent>() const;
	template std::vector<Entity>	ComponentBase::GetComponentsInChildren<ConvexMeshColliderComponent>() const;
	template std::vector<Entity>	ComponentBase::GetComponentsInChildren<HeightFieldColliderComponent>() const;
	template std::vector<Entity>	ComponentBase::GetComponentsInChildren<TriangleMeshColliderComponent>() const;

#pragma endregion

    namespace Util
    {
        std::string ComponentTypeToString(ComponentType type)
        {
            switch (type)
            {
                case ComponentType::TransformComponent:             return "TransformComponent";
                case ComponentType::MetaComponent:                  return "MetaComponent";

                case ComponentType::CameraComponent:                return "CameraComponent";

                case ComponentType::AudioListenerComponent:         return "AudioListenerComponent";
                case ComponentType::AudioEmitterComponent:          return "AudioEmitterComponent";

                case ComponentType::LODMeshComponent:               return "LODMeshComponent";
                case ComponentType::MeshRendererComponent:          return "MeshRendererComponent";
                case ComponentType::SkinnedMeshRendererComponent:   return "SkinnedMeshRendererComponent";

                case ComponentType::SimpleAnimationComponent:       return "SimpleAnimationComponent";
                case ComponentType::AnimatorComponent:              return "AnimatorComponent";

                case ComponentType::SphereLightComponent:           return "SphereLightComponent";
                case ComponentType::ConeLightComponent:             return "ConeLightComponent";
                case ComponentType::DirectionalLightComponent:      return "DirectionalLightComponent";

                case ComponentType::RigidbodyComponent:             return "RigidbodyComponent";

                case ComponentType::PlaneCollider:                  return "PlaneCollider";
                case ComponentType::BoxColliderComponent:           return "BoxColliderComponent";
                case ComponentType::SphereColliderComponent:        return "SphereColliderComponent";
                case ComponentType::CapsuleColliderComponent:       return "CapsuleColliderComponent";

                case ComponentType::ConvexMeshColliderComponent:    return "ConvexMeshColliderComponent";
                case ComponentType::HeightFieldColliderComponent:   return "HeightFieldColliderComponent";
                case ComponentType::TriangleMeshColliderComponent:  return "TriangleMeshColliderComponent";

            }
            return "Unknown";
        }

        ComponentType ComponentTypeFromString(const std::string &type_string)
        {
            static const std::unordered_map<std::string, ComponentType> string_to_type_map = {
                {"TransformComponent",              ComponentType::TransformComponent},
                {"MetaComponent",                   ComponentType::MetaComponent},

                {"CameraComponent",                 ComponentType::CameraComponent},

                {"AudioListenerComponent",          ComponentType::AudioListenerComponent},
                {"AudioEmitterComponent",           ComponentType::AudioEmitterComponent},

                {"LODMeshComponent",                ComponentType::LODMeshComponent},
                {"MeshRendererComponent",           ComponentType::MeshRendererComponent},
                {"SkinnedMeshRendererComponent",    ComponentType::SkinnedMeshRendererComponent},

                {"SimpleAnimationComponent",        ComponentType::SimpleAnimationComponent},
                {"AnimatorComponent",               ComponentType::AnimatorComponent},

                {"SphereLightComponent",            ComponentType::SphereLightComponent},
                {"ConeLightComponent",              ComponentType::ConeLightComponent},
                {"DirectionalLightComponent",       ComponentType::DirectionalLightComponent},

                {"RigidbodyComponent",              ComponentType::RigidbodyComponent},

                {"PlaneCollider",                   ComponentType::PlaneCollider},
                {"BoxColliderComponent",            ComponentType::BoxColliderComponent},
                {"SphereColliderComponent",         ComponentType::SphereColliderComponent},
                {"CapsuleColliderComponent",        ComponentType::CapsuleColliderComponent},

                {"ConvexMeshColliderComponent",     ComponentType::ConvexMeshColliderComponent},
                {"HeightFieldColliderComponent",    ComponentType::HeightFieldColliderComponent},
                {"TriangleMeshColliderComponent",   ComponentType::TriangleMeshColliderComponent}
            };

            auto it = string_to_type_map.find(type_string);
            if (it != string_to_type_map.end())
                return it->second;

            BC_CORE_WARN("ComponentTypeFromString: Unknown Component String - '{}'", type_string);
            return ComponentType::Unknown;
        }
    }

}
