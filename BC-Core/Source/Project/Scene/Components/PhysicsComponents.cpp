#include "BC_PCH.h"
#include "PhysicsComponents.h"

// TODO: Implement

namespace BC
{
    
#pragma region RigidbodyComponent
    
    RigidbodyComponent::RigidbodyComponent(const RigidbodyComponent& other)
    {

    }

    RigidbodyComponent::RigidbodyComponent(RigidbodyComponent&& other) noexcept
    {

    }

    RigidbodyComponent& RigidbodyComponent::operator=(const RigidbodyComponent& other)
    {
        if (this == &other)
            return *this;

        return *this;
    }

    RigidbodyComponent& RigidbodyComponent::operator=(RigidbodyComponent&& other) noexcept
    {
        if (this == &other)
            return *this;

        return *this;
    }

    bool RigidbodyComponent::IsValid() const
    {
        return false;
    }

    void RigidbodyComponent::SceneSerialise(YAML::Emitter& out) const
    {

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

        return *this;
    }

    void PlaneCollider::SceneSerialise(YAML::Emitter& out) const
    {

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

        return *this;
    }

    void BoxColliderComponent::SceneSerialise(YAML::Emitter& out) const
    {

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

        return *this;
    }

    void SphereColliderComponent::SceneSerialise(YAML::Emitter& out) const
    {

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

        return *this;
    }

    void CapsuleColliderComponent::SceneSerialise(YAML::Emitter& out) const
    {

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

        return *this;
    }

    void ConvexMeshColliderComponent::SceneSerialise(YAML::Emitter& out) const
    {

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

        return *this;
    }

    void HeightFieldColliderComponent::SceneSerialise(YAML::Emitter& out) const
    {

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

        return *this;
    }

    void TriangleMeshColliderComponent::SceneSerialise(YAML::Emitter& out) const
    {

    }

    bool TriangleMeshColliderComponent::SceneDeserialise(const YAML::Node& data)
    {
        return false;
    }

#pragma endregion

}
