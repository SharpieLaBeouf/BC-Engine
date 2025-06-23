#pragma once

// Core Headers
#include "Core/GUID.h"

// C++ Standard Library Headers
#include <vector>
#include <memory>
#include <string>

// External Vendor Library Headers

namespace YAML 
{
    class Emitter;
    class Node;
}

namespace BC
{

    enum class ComponentType : uint8_t
    {
        // Default
        TransformComponent,
        MetaComponent,
        
        // Camera
        CameraComponent,
        
        // Audio
        AudioListenerComponent,
        AudioEmitterComponent,

        // Meshes
        LODMeshComponent,
        MeshRendererComponent,
        SkinnedMeshRendererComponent,

        // Animation
        SimpleAnimationComponent,
        AnimatorComponent,

        // Lighting
        SphereLightComponent,
        ConeLightComponent,
        DirectionalLightComponent,

        // Physics
        RigidbodyComponent,

        PlaneCollider,
        BoxColliderComponent,
        SphereColliderComponent,
        CapsuleColliderComponent,
        
        ConvexMeshColliderComponent,
        HeightFieldColliderComponent,
        TriangleMeshColliderComponent,
        
        Unknown
    };

    class Entity;

    struct ComponentBase;

    struct TransformComponent;
    struct MetaComponent;

    struct CameraComponent;

    struct AudioListenerComponent;
    struct AudioEmitterComponent;

    struct LODMeshComponent;
    struct MeshRendererComponent;
    struct SkinnedMeshRendererComponent;

    struct SimpleAnimationComponent;
    struct AnimatorComponent;

    struct SphereLightComponent;
    struct ConeLightComponent;
    struct DirectionalLightComponent;

    struct RigidbodyComponent;

    struct PlaneCollider;
    struct BoxColliderComponent;
    struct SphereColliderComponent;
    struct CapsuleColliderComponent;

    struct ConvexMeshColliderComponent;
    struct HeightFieldColliderComponent;
    struct TriangleMeshColliderComponent;

    namespace Util
    {

    #pragma region Component Group

        template<typename... Component>
        struct ComponentGroup {};
        
        using AllComponents = ComponentGroup <

            ComponentBase,

            // Default
            TransformComponent,
            MetaComponent,
            
            // Camera
            CameraComponent,
            
            // Audio
            AudioListenerComponent,
            AudioEmitterComponent,

            // Meshes
            LODMeshComponent,
            MeshRendererComponent,
            SkinnedMeshRendererComponent,

            // Animation
            SimpleAnimationComponent,
            AnimatorComponent,

            // Lighting
            SphereLightComponent,
            ConeLightComponent,
            DirectionalLightComponent,

            // Physics
            RigidbodyComponent,

            PlaneCollider,
            BoxColliderComponent,
            SphereColliderComponent,
            CapsuleColliderComponent,
            
            ConvexMeshColliderComponent,
            HeightFieldColliderComponent,
            TriangleMeshColliderComponent

        >;

    #pragma endregion

    #pragma region Component Type to and From String
    
        std::string ComponentTypeToString(ComponentType type);
        ComponentType ComponentTypeFromString(const std::string& type_string);

    #pragma endregion

    }

    // -----Move/Copy Rules-----
    // When Copy -> do not copy m_EntityID, only component data, and perform initialisations as required
    // When Move -> move m_EntityID into new component, and reset other to default state
    // -----Move/Copy Rules-----

    struct ComponentBase 
    {

    public:

        virtual ComponentType GetType() = 0;
        virtual void SceneSerialise(YAML::Emitter& out) const = 0;
        virtual bool SceneDeserialise(const YAML::Node& data) = 0;

        virtual ~ComponentBase() = default;

        // Init() and Shutdown()
        //
        // To be overriden by classes that have initialisation and shutdown
        // requirements, e.g., physics components (creating shapes, actors,
        // etc.), camera component (render target asset creation and
        // desctruction), etc.

        virtual bool Init()     { return true; } 
        virtual bool Shutdown() { return true; }

        GUID GetGUID() const;
        void SetGUID(GUID entity_guid);

        Entity GetEntity() const;

        template<typename T>
        T& GetComponent() const;

        template<typename T>
        T& GetComponentInParent() const;

        template<typename T>
        T& GetComponentInChild() const;

        template<typename T>
        std::vector<Entity> GetComponentsInParents() const;

        template<typename T>
        std::vector<Entity> GetComponentsInChildren() const;

    protected:

        GUID m_EntityID = NULL_GUID;

    };

}