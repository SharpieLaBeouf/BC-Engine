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
        ~RigidbodyComponent()
        {
            Shutdown();
        }

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
    
    struct BoxColliderComponent : public ComponentBase
    {
    
    public:

        BoxColliderComponent() = default;
        ~BoxColliderComponent()
        {
            Shutdown();
        }

        BoxColliderComponent(const BoxColliderComponent& other);
        BoxColliderComponent(BoxColliderComponent&& other) noexcept;
        BoxColliderComponent& operator=(const BoxColliderComponent& other);
        BoxColliderComponent& operator=(BoxColliderComponent&& other) noexcept;

        ComponentType GetType() const override { return ComponentType::BoxColliderComponent; }

        bool Init() override;
        bool Shutdown() override;

        void SceneSerialise(YAML::Emitter& out) const override;
        bool SceneDeserialise(const YAML::Node& data) override;

        BoxPhysicsShape* GetShape() { return &m_Shape; }

    private:

        BoxPhysicsShape m_Shape;

    };
    
    struct SphereColliderComponent : public ComponentBase
    {
    
    public:

        SphereColliderComponent() = default;
        ~SphereColliderComponent()
        {
            Shutdown();
        }

        SphereColliderComponent(const SphereColliderComponent& other);
        SphereColliderComponent(SphereColliderComponent&& other) noexcept;
        SphereColliderComponent& operator=(const SphereColliderComponent& other);
        SphereColliderComponent& operator=(SphereColliderComponent&& other) noexcept;

        ComponentType GetType() const override { return ComponentType::SphereColliderComponent; }

        bool Init() override;
        bool Shutdown() override;

        void SceneSerialise(YAML::Emitter& out) const override;
        bool SceneDeserialise(const YAML::Node& data) override;

        SpherePhysicsShape* GetShape() { return &m_Shape; }

    private:

        SpherePhysicsShape m_Shape;

    };
    
    struct CapsuleColliderComponent : public ComponentBase
    {
    
    public:

        CapsuleColliderComponent() = default;
        ~CapsuleColliderComponent()
        {
            Shutdown();
        }

        CapsuleColliderComponent(const CapsuleColliderComponent& other);
        CapsuleColliderComponent(CapsuleColliderComponent&& other) noexcept;
        CapsuleColliderComponent& operator=(const CapsuleColliderComponent& other);
        CapsuleColliderComponent& operator=(CapsuleColliderComponent&& other) noexcept;

        ComponentType GetType() const override { return ComponentType::CapsuleColliderComponent; }

        bool Init() override;
        bool Shutdown() override;

        void SceneSerialise(YAML::Emitter& out) const override;
        bool SceneDeserialise(const YAML::Node& data) override;

        CapsulePhysicsShape* GetShape() { return &m_Shape; }

    private:

        CapsulePhysicsShape m_Shape;

    };
    
    struct ConvexMeshColliderComponent : public ComponentBase
    {
    
    public:

        ConvexMeshColliderComponent() = default;
        ~ConvexMeshColliderComponent()
        {
            Shutdown();
        }

        ConvexMeshColliderComponent(const ConvexMeshColliderComponent& other);
        ConvexMeshColliderComponent(ConvexMeshColliderComponent&& other) noexcept;
        ConvexMeshColliderComponent& operator=(const ConvexMeshColliderComponent& other);
        ConvexMeshColliderComponent& operator=(ConvexMeshColliderComponent&& other) noexcept;

        ComponentType GetType() const override { return ComponentType::ConvexMeshColliderComponent; }

        bool Init() override;
        bool Shutdown() override;

        void SceneSerialise(YAML::Emitter& out) const override;
        bool SceneDeserialise(const YAML::Node& data) override;

        ConvexMeshPhysicsShape* GetShape() { return &m_Shape; }

    private:

        ConvexMeshPhysicsShape m_Shape;

    };
    
    struct HeightFieldColliderComponent : public ComponentBase
    {
    
    public:

        HeightFieldColliderComponent() = default;
        ~HeightFieldColliderComponent()
        {
            Shutdown();
        }

        HeightFieldColliderComponent(const HeightFieldColliderComponent& other);
        HeightFieldColliderComponent(HeightFieldColliderComponent&& other) noexcept;
        HeightFieldColliderComponent& operator=(const HeightFieldColliderComponent& other);
        HeightFieldColliderComponent& operator=(HeightFieldColliderComponent&& other) noexcept;

        ComponentType GetType() const override { return ComponentType::HeightFieldColliderComponent; }

        bool Init() override;
        bool Shutdown() override;

        void SceneSerialise(YAML::Emitter& out) const override;
        bool SceneDeserialise(const YAML::Node& data) override;

        HeightFieldPhysicsShape* GetShape() { return &m_Shape; }

    private:

        HeightFieldPhysicsShape m_Shape;

    };
    
    struct TriangleMeshColliderComponent : public ComponentBase
    {
    
    public:

        TriangleMeshColliderComponent() = default;
        ~TriangleMeshColliderComponent()
        {
            Shutdown();
        }

        TriangleMeshColliderComponent(const TriangleMeshColliderComponent& other);
        TriangleMeshColliderComponent(TriangleMeshColliderComponent&& other) noexcept;
        TriangleMeshColliderComponent& operator=(const TriangleMeshColliderComponent& other);
        TriangleMeshColliderComponent& operator=(TriangleMeshColliderComponent&& other) noexcept;

        ComponentType GetType() const override { return ComponentType::TriangleMeshColliderComponent; }

        bool Init() override;
        bool Shutdown() override;

        void SceneSerialise(YAML::Emitter& out) const override;
        bool SceneDeserialise(const YAML::Node& data) override;

        TriangleMeshPhysicsShape* GetShape() { return &m_Shape; }

    private:

        TriangleMeshPhysicsShape m_Shape;

    };

}