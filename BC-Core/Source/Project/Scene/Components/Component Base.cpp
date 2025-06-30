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
    T* ComponentBase::TryGetComponent() const
    {
        if (!m_Entity)
        {
            return nullptr;
        }
        return m_Entity->TryGetComponent<T>();
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
                case ComponentType::TransformComponent:             return "Transform Component";
                case ComponentType::MetaComponent:                  return "Meta Component";

                case ComponentType::CameraComponent:                return "Camera Component";

                case ComponentType::AudioListenerComponent:         return "Audio Listener Component";
                case ComponentType::AudioEmitterComponent:          return "Audio Emitter Component";

                case ComponentType::LODMeshComponent:               return "LOD Mesh Component";
                case ComponentType::MeshRendererComponent:          return "Mesh Renderer Component";
                case ComponentType::SkinnedMeshRendererComponent:   return "Skinned Mesh Renderer Component";

                case ComponentType::SimpleAnimationComponent:       return "Simple Animation Component";
                case ComponentType::AnimatorComponent:              return "Animator Component";

                case ComponentType::SphereLightComponent:           return "Sphere Light Component";
                case ComponentType::ConeLightComponent:             return "Cone Light Component";
                case ComponentType::DirectionalLightComponent:      return "Directional Light Component";

                case ComponentType::RigidbodyComponent:             return "Rigidbody Component";
                
                case ComponentType::BoxColliderComponent:           return "Box Collider Component";
                case ComponentType::SphereColliderComponent:        return "Sphere Collider Component";
                case ComponentType::CapsuleColliderComponent:       return "Capsule Collider Component";

                case ComponentType::ConvexMeshColliderComponent:    return "Convex Mesh Collider Component";
                case ComponentType::HeightFieldColliderComponent:   return "Height Field Collider Component";
                case ComponentType::TriangleMeshColliderComponent:  return "Triangle Mesh Collider Component";

            }
            return "Unknown";
        }

        ComponentType ComponentTypeFromString(const std::string &type_string)
        {
            static const std::unordered_map<std::string, ComponentType> string_to_type_map = 
            {
                {"Transform Component",                 ComponentType::TransformComponent},
                {"Meta Component",                      ComponentType::MetaComponent},

                {"Camera Component",                    ComponentType::CameraComponent},

                {"Audio Listener Component",            ComponentType::AudioListenerComponent},
                {"Audio Emitter Component",             ComponentType::AudioEmitterComponent},

                {"LOD Mesh Component",                  ComponentType::LODMeshComponent},
                {"Mesh Renderer Component",             ComponentType::MeshRendererComponent},
                {"Skinned Mesh Renderer Component",      ComponentType::SkinnedMeshRendererComponent},

                {"Simple Animation Component",          ComponentType::SimpleAnimationComponent},
                {"Animator Component",                  ComponentType::AnimatorComponent},

                {"Sphere Light Component",              ComponentType::SphereLightComponent},
                {"Cone Light Component",                ComponentType::ConeLightComponent},
                {"Directional Light Component",         ComponentType::DirectionalLightComponent},

                {"Rigidbody Component",                 ComponentType::RigidbodyComponent},
                
                {"Box Collider Component",              ComponentType::BoxColliderComponent},
                {"Sphere Collider Component",           ComponentType::SphereColliderComponent},
                {"Capsule Collider Component",          ComponentType::CapsuleColliderComponent},

                {"Convex MeshCollider Component",       ComponentType::ConvexMeshColliderComponent},
                {"Height FieldCollider Component",      ComponentType::HeightFieldColliderComponent},
                {"Triangle MeshCollider Component",     ComponentType::TriangleMeshColliderComponent}
            };

            auto it = string_to_type_map.find(type_string);
            if (it != string_to_type_map.end())
                return it->second;

            BC_CORE_WARN("ComponentTypeFromString: Unknown Component String - '{}'", type_string);
            return ComponentType::Unknown;
        }
    }

}
