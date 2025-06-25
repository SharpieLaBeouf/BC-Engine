#pragma once

// Core Headers
#include "Component Base.h"

#include "Physics/PhysicsWrapper.h"

// C++ Standard Library Headers

// External Vendor Library Headers

namespace YAML 
{
	class Emitter;
	class Node;
}

namespace BC
{

    struct RigidbodyComponent : public ComponentBase
    {
    
    public:

        RigidbodyComponent() = default;
        ~RigidbodyComponent() = default;

        RigidbodyComponent(const RigidbodyComponent& other);
        RigidbodyComponent(RigidbodyComponent&& other) noexcept;
        RigidbodyComponent& operator=(const RigidbodyComponent& other);
        RigidbodyComponent& operator=(RigidbodyComponent&& other) noexcept;

        ComponentType GetType() const override { return ComponentType::RigidbodyComponent; }

        bool Init() override;
        bool Shutdown() override;

        void SceneSerialise(YAML::Emitter& out) const override;
        bool SceneDeserialise(const YAML::Node& data) override;

        RigidDynamic* GetRigid() { return &m_Rigid; }

    private:
        
        RigidDynamic m_Rigid;

    };
    
    struct PlaneCollider : public ComponentBase
    {
    
    public:

        PlaneCollider() = default;
        ~PlaneCollider() = default;

        PlaneCollider(const PlaneCollider& other);
        PlaneCollider(PlaneCollider&& other) noexcept;
        PlaneCollider& operator=(const PlaneCollider& other);
        PlaneCollider& operator=(PlaneCollider&& other) noexcept;

        ComponentType GetType() const override { return ComponentType::PlaneCollider; }

        bool Init() override;
        bool Shutdown() override;

        void SceneSerialise(YAML::Emitter& out) const override;
        bool SceneDeserialise(const YAML::Node& data) override;

    private:
        

    };
    
    struct BoxColliderComponent : public ComponentBase
    {
    
    public:

        BoxColliderComponent() = default;
        ~BoxColliderComponent() = default;

        BoxColliderComponent(const BoxColliderComponent& other);
        BoxColliderComponent(BoxColliderComponent&& other) noexcept;
        BoxColliderComponent& operator=(const BoxColliderComponent& other);
        BoxColliderComponent& operator=(BoxColliderComponent&& other) noexcept;

        ComponentType GetType() const override { return ComponentType::BoxColliderComponent; }

        bool Init() override;
        bool Shutdown() override;

        void SceneSerialise(YAML::Emitter& out) const override;
        bool SceneDeserialise(const YAML::Node& data) override;

    private:
        

    };
    
    struct SphereColliderComponent : public ComponentBase
    {
    
    public:

        SphereColliderComponent() = default;
        ~SphereColliderComponent() = default;

        SphereColliderComponent(const SphereColliderComponent& other);
        SphereColliderComponent(SphereColliderComponent&& other) noexcept;
        SphereColliderComponent& operator=(const SphereColliderComponent& other);
        SphereColliderComponent& operator=(SphereColliderComponent&& other) noexcept;

        ComponentType GetType() const override { return ComponentType::SphereColliderComponent; }

        bool Init() override;
        bool Shutdown() override;

        void SceneSerialise(YAML::Emitter& out) const override;
        bool SceneDeserialise(const YAML::Node& data) override;

    private:
        

    };
    
    struct CapsuleColliderComponent : public ComponentBase
    {
    
    public:

        CapsuleColliderComponent() = default;
        ~CapsuleColliderComponent() = default;

        CapsuleColliderComponent(const CapsuleColliderComponent& other);
        CapsuleColliderComponent(CapsuleColliderComponent&& other) noexcept;
        CapsuleColliderComponent& operator=(const CapsuleColliderComponent& other);
        CapsuleColliderComponent& operator=(CapsuleColliderComponent&& other) noexcept;

        ComponentType GetType() const override { return ComponentType::CapsuleColliderComponent; }

        bool Init() override;
        bool Shutdown() override;

        void SceneSerialise(YAML::Emitter& out) const override;
        bool SceneDeserialise(const YAML::Node& data) override;

    private:
        

    };
    
    struct ConvexMeshColliderComponent : public ComponentBase
    {
    
    public:

        ConvexMeshColliderComponent() = default;
        ~ConvexMeshColliderComponent() = default;

        ConvexMeshColliderComponent(const ConvexMeshColliderComponent& other);
        ConvexMeshColliderComponent(ConvexMeshColliderComponent&& other) noexcept;
        ConvexMeshColliderComponent& operator=(const ConvexMeshColliderComponent& other);
        ConvexMeshColliderComponent& operator=(ConvexMeshColliderComponent&& other) noexcept;

        ComponentType GetType() const override { return ComponentType::ConvexMeshColliderComponent; }

        bool Init() override;
        bool Shutdown() override;

        void SceneSerialise(YAML::Emitter& out) const override;
        bool SceneDeserialise(const YAML::Node& data) override;

    private:
        

    };
    
    struct HeightFieldColliderComponent : public ComponentBase
    {
    
    public:

        HeightFieldColliderComponent() = default;
        ~HeightFieldColliderComponent() = default;

        HeightFieldColliderComponent(const HeightFieldColliderComponent& other);
        HeightFieldColliderComponent(HeightFieldColliderComponent&& other) noexcept;
        HeightFieldColliderComponent& operator=(const HeightFieldColliderComponent& other);
        HeightFieldColliderComponent& operator=(HeightFieldColliderComponent&& other) noexcept;

        ComponentType GetType() const override { return ComponentType::HeightFieldColliderComponent; }

        bool Init() override;
        bool Shutdown() override;

        void SceneSerialise(YAML::Emitter& out) const override;
        bool SceneDeserialise(const YAML::Node& data) override;

    private:
        

    };
    
    struct TriangleMeshColliderComponent : public ComponentBase
    {
    
    public:

        TriangleMeshColliderComponent() = default;
        ~TriangleMeshColliderComponent() = default;

        TriangleMeshColliderComponent(const TriangleMeshColliderComponent& other);
        TriangleMeshColliderComponent(TriangleMeshColliderComponent&& other) noexcept;
        TriangleMeshColliderComponent& operator=(const TriangleMeshColliderComponent& other);
        TriangleMeshColliderComponent& operator=(TriangleMeshColliderComponent&& other) noexcept;

        ComponentType GetType() const override { return ComponentType::TriangleMeshColliderComponent; }

        bool Init() override;
        bool Shutdown() override;

        void SceneSerialise(YAML::Emitter& out) const override;
        bool SceneDeserialise(const YAML::Node& data) override;

    private:
        

    };

}