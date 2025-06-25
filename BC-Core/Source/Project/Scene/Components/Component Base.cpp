#include "BC_PCH.h"
#include "Component Base.h"

#include "Project/Scene/Entity.h"

namespace BC
{

#pragma region Component Base Functions

    Entity ComponentBase::GetEntity() const
    {
        return *m_Entity.get();
    }

    void ComponentBase::SetEntity(const Entity& entity)
    {
        m_Entity.reset();
        m_Entity = std::make_shared<Entity>(entity);
    }

    template <typename T>
    T& ComponentBase::GetComponent() const
    {
        if (!m_Entity)
        {
            return Entity::GetBlankComponent<T>();
        }
        return m_Entity->GetComponent<T>();
    }

    template <typename T>
    T& ComponentBase::GetComponentInParent() const
    {
        if (!m_Entity)
        {
            return Entity::GetBlankComponent<T>();
        }
        return m_Entity->GetComponentInParent<T>();
    }

    template <typename T>
    T& ComponentBase::GetComponentInChild() const
    {
        if (!m_Entity)
        {
            return Entity::GetBlankComponent<T>();
        }
        return m_Entity->GetComponentInChild<T>();
    }

    template <typename T>
    std::vector<Entity> ComponentBase::GetComponentsInParents() const
    {
        if (!m_Entity)
        {
            return std::vector<Entity>();
        }
        return m_Entity->GetComponentsInParents<T>();
    }

    template <typename T>
    std::vector<Entity> ComponentBase::GetComponentsInChildren() const
    {
        if (!m_Entity)
        {
            return std::vector<Entity>();
        }
        return m_Entity->GetComponentsInChildren<T>();
    }

#pragma endregion

#pragma region Component Base Function Template Specialisations


#define INSTANTIATE_COMPONENT_BASE_TEMPLATES(T) \
    template T& ComponentBase::GetComponent<T>() const; \
    template T& ComponentBase::GetComponentInParent<T>() const; \
    template T& ComponentBase::GetComponentInChild<T>() const; \
    template std::vector<Entity> ComponentBase::GetComponentsInParents<T>() const; \
    template std::vector<Entity> ComponentBase::GetComponentsInChildren<T>() const;

EXPAND_COMPONENTS(INSTANTIATE_COMPONENT_BASE_TEMPLATES);

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
