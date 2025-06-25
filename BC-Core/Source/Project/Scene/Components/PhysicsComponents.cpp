#include "BC_PCH.h"
#include "PhysicsComponents.h"

#include <yaml-cpp/yaml.h>

// TODO: Implement

namespace BC
{
    
#pragma region RigidbodyComponent
    
    RigidbodyComponent::RigidbodyComponent(const RigidbodyComponent& other)
    {
        m_Rigid.SetProperties(other.m_Rigid.GetProperties());
    }

    RigidbodyComponent::RigidbodyComponent(RigidbodyComponent&& other) noexcept
    {
		m_Entity = std::move(other.m_Entity); other.m_Entity = nullptr;

        m_Rigid = std::move(other.m_Rigid); other.m_Rigid = {};
    }

    RigidbodyComponent& RigidbodyComponent::operator=(const RigidbodyComponent& other)
    {
        if (this == &other)
            return *this;

        m_Rigid.SetProperties(other.m_Rigid.GetProperties());

        return *this;
    }

    RigidbodyComponent& RigidbodyComponent::operator=(RigidbodyComponent&& other) noexcept
    {
        if (this == &other)
            return *this;

        m_Entity = std::move(other.m_Entity); other.m_Entity = nullptr;

        m_Rigid = std::move(other.m_Rigid); other.m_Rigid = {};

        return *this;
    }
    
    bool RigidbodyComponent::Init()
    {
        return false;
    }

    bool RigidbodyComponent::Shutdown()
    {
        return false;
    }

    void RigidbodyComponent::SceneSerialise(YAML::Emitter& out) const
    {
        out << YAML::Key << Util::ComponentTypeToString(GetType()) << YAML::Value;
		out << YAML::BeginMap;
		{
            
        }
		out << YAML::EndMap;
    }

    bool RigidbodyComponent::SceneDeserialise(const YAML::Node& data)
    {
        return false;
    }

#pragma endregion
    
#pragma region PlaneCollider
    
    PlaneCollider::PlaneCollider(const PlaneCollider& other)
    {

    }

    PlaneCollider::PlaneCollider(PlaneCollider&& other) noexcept
    {
        m_Entity = std::move(other.m_Entity); other.m_Entity = nullptr;

    }

    PlaneCollider& PlaneCollider::operator=(const PlaneCollider& other)
    {
        if (this == &other)
            return *this;

        return *this;
    }

    PlaneCollider& PlaneCollider::operator=(PlaneCollider&& other) noexcept
    {
        if (this == &other)
            return *this;

        m_Entity = std::move(other.m_Entity); other.m_Entity = nullptr;

        return *this;
    }
    
    bool PlaneCollider::Init()
    {
        return false;
    }

    bool PlaneCollider::Shutdown()
    {
        return false;
    }

    void PlaneCollider::SceneSerialise(YAML::Emitter& out) const
    {
        out << YAML::Key << Util::ComponentTypeToString(GetType()) << YAML::Value;
		out << YAML::BeginMap;
		{
            
        }
		out << YAML::EndMap;
    }

    bool PlaneCollider::SceneDeserialise(const YAML::Node& data)
    {
        return false;
    }

#pragma endregion

#pragma region BoxColliderComponent
    
    BoxColliderComponent::BoxColliderComponent(const BoxColliderComponent& other)
    {

    }

    BoxColliderComponent::BoxColliderComponent(BoxColliderComponent&& other) noexcept
    {

        m_Entity = std::move(other.m_Entity); other.m_Entity = nullptr;

    }

    BoxColliderComponent& BoxColliderComponent::operator=(const BoxColliderComponent& other)
    {
        if (this == &other)
            return *this;

        return *this;
    }

    BoxColliderComponent& BoxColliderComponent::operator=(BoxColliderComponent&& other) noexcept
    {
        if (this == &other)
            return *this;

        m_Entity = std::move(other.m_Entity); other.m_Entity = nullptr;

        return *this;
    }
    
    bool BoxColliderComponent::Init()
    {
        return false;
    }

    bool BoxColliderComponent::Shutdown()
    {
        return false;
    }

    void BoxColliderComponent::SceneSerialise(YAML::Emitter& out) const
    {
        out << YAML::Key << Util::ComponentTypeToString(GetType()) << YAML::Value;
		out << YAML::BeginMap;
		{
            
        }
		out << YAML::EndMap;
    }

    bool BoxColliderComponent::SceneDeserialise(const YAML::Node& data)
    {
        return false;
    }

#pragma endregion

#pragma region SphereColliderComponent
    
    SphereColliderComponent::SphereColliderComponent(const SphereColliderComponent& other)
    {

    }

    SphereColliderComponent::SphereColliderComponent(SphereColliderComponent&& other) noexcept
    {
        m_Entity = std::move(other.m_Entity); other.m_Entity = nullptr;

    }

    SphereColliderComponent& SphereColliderComponent::operator=(const SphereColliderComponent& other)
    {
        if (this == &other)
            return *this;

        return *this;
    }

    SphereColliderComponent& SphereColliderComponent::operator=(SphereColliderComponent&& other) noexcept
    {
        if (this == &other)
            return *this;

        m_Entity = std::move(other.m_Entity); other.m_Entity = nullptr;

        return *this;
    }
    
    bool SphereColliderComponent::Init()
    {
        return false;
    }

    bool SphereColliderComponent::Shutdown()
    {
        return false;
    }

    void SphereColliderComponent::SceneSerialise(YAML::Emitter& out) const
    {
        out << YAML::Key << Util::ComponentTypeToString(GetType()) << YAML::Value;
		out << YAML::BeginMap;
		{
            
        }
		out << YAML::EndMap;
    }

    bool SphereColliderComponent::SceneDeserialise(const YAML::Node& data)
    {
        return false;
    }

#pragma endregion

#pragma region CapsuleColliderComponent
    
    CapsuleColliderComponent::CapsuleColliderComponent(const CapsuleColliderComponent& other)
    {

    }

    CapsuleColliderComponent::CapsuleColliderComponent(CapsuleColliderComponent&& other) noexcept
    {
        m_Entity = std::move(other.m_Entity); other.m_Entity = nullptr;

    }

    CapsuleColliderComponent& CapsuleColliderComponent::operator=(const CapsuleColliderComponent& other)
    {
        if (this == &other)
            return *this;

        return *this;
    }

    CapsuleColliderComponent& CapsuleColliderComponent::operator=(CapsuleColliderComponent&& other) noexcept
    {
        if (this == &other)
            return *this;

        m_Entity = std::move(other.m_Entity); other.m_Entity = nullptr;

        return *this;
    }
    
    bool CapsuleColliderComponent::Init()
    {
        return false;
    }

    bool CapsuleColliderComponent::Shutdown()
    {
        return false;
    }

    void CapsuleColliderComponent::SceneSerialise(YAML::Emitter& out) const
    {
        out << YAML::Key << Util::ComponentTypeToString(GetType()) << YAML::Value;
		out << YAML::BeginMap;
		{
            
        }
		out << YAML::EndMap;
    }

    bool CapsuleColliderComponent::SceneDeserialise(const YAML::Node& data)
    {
        return false;
    }

#pragma endregion

#pragma region ConvexMeshColliderComponent
    
    ConvexMeshColliderComponent::ConvexMeshColliderComponent(const ConvexMeshColliderComponent& other)
    {

    }

    ConvexMeshColliderComponent::ConvexMeshColliderComponent(ConvexMeshColliderComponent&& other) noexcept
    {
        m_Entity = std::move(other.m_Entity); other.m_Entity = nullptr;

    }

    ConvexMeshColliderComponent& ConvexMeshColliderComponent::operator=(const ConvexMeshColliderComponent& other)
    {
        if (this == &other)
            return *this;

        return *this;
    }

    ConvexMeshColliderComponent& ConvexMeshColliderComponent::operator=(ConvexMeshColliderComponent&& other) noexcept
    {
        if (this == &other)
            return *this;

        m_Entity = std::move(other.m_Entity); other.m_Entity = nullptr;

        return *this;
    }
    
    bool ConvexMeshColliderComponent::Init()
    {
        return false;
    }

    bool ConvexMeshColliderComponent::Shutdown()
    {
        return false;
    }

    void ConvexMeshColliderComponent::SceneSerialise(YAML::Emitter& out) const
    {
        out << YAML::Key << Util::ComponentTypeToString(GetType()) << YAML::Value;
		out << YAML::BeginMap;
		{
            
        }
		out << YAML::EndMap;
    }

    bool ConvexMeshColliderComponent::SceneDeserialise(const YAML::Node& data)
    {
        return false;
    }

#pragma endregion

#pragma region HeightFieldColliderComponent
    
    HeightFieldColliderComponent::HeightFieldColliderComponent(const HeightFieldColliderComponent& other)
    {

    }

    HeightFieldColliderComponent::HeightFieldColliderComponent(HeightFieldColliderComponent&& other) noexcept
    {
        m_Entity = std::move(other.m_Entity); other.m_Entity = nullptr;

    }

    HeightFieldColliderComponent& HeightFieldColliderComponent::operator=(const HeightFieldColliderComponent& other)
    {
        if (this == &other)
            return *this;

        return *this;
    }

    HeightFieldColliderComponent& HeightFieldColliderComponent::operator=(HeightFieldColliderComponent&& other) noexcept
    {
        if (this == &other)
            return *this;

        m_Entity = std::move(other.m_Entity); other.m_Entity = nullptr;

        return *this;
    }
    
    bool HeightFieldColliderComponent::Init()
    {
        return false;
    }

    bool HeightFieldColliderComponent::Shutdown()
    {
        return false;
    }

    void HeightFieldColliderComponent::SceneSerialise(YAML::Emitter& out) const
    {
        out << YAML::Key << Util::ComponentTypeToString(GetType()) << YAML::Value;
		out << YAML::BeginMap;
		{
            
        }
		out << YAML::EndMap;
    }

    bool HeightFieldColliderComponent::SceneDeserialise(const YAML::Node& data)
    {
        return false;
    }

#pragma endregion

#pragma region TriangleMeshColliderComponent
    
    TriangleMeshColliderComponent::TriangleMeshColliderComponent(const TriangleMeshColliderComponent& other)
    {

    }

    TriangleMeshColliderComponent::TriangleMeshColliderComponent(TriangleMeshColliderComponent&& other) noexcept
    {
        m_Entity = std::move(other.m_Entity); other.m_Entity = nullptr;

    }

    TriangleMeshColliderComponent& TriangleMeshColliderComponent::operator=(const TriangleMeshColliderComponent& other)
    {
        if (this == &other)
            return *this;

        return *this;
    }

    TriangleMeshColliderComponent& TriangleMeshColliderComponent::operator=(TriangleMeshColliderComponent&& other) noexcept
    {
        if (this == &other)
            return *this;
            
        m_Entity = std::move(other.m_Entity); other.m_Entity = nullptr;

        return *this;
    }
    
    bool TriangleMeshColliderComponent::Init()
    {
        return false;
    }

    bool TriangleMeshColliderComponent::Shutdown()
    {
        return false;
    }

    void TriangleMeshColliderComponent::SceneSerialise(YAML::Emitter& out) const
    {
        out << YAML::Key << Util::ComponentTypeToString(GetType()) << YAML::Value;
		out << YAML::BeginMap;
		{
            
        }
		out << YAML::EndMap;
    }

    bool TriangleMeshColliderComponent::SceneDeserialise(const YAML::Node& data)
    {
        return false;
    }

#pragma endregion

}
