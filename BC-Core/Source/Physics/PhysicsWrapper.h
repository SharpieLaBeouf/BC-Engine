#pragma once

#include "Asset/Asset.h"
#include "Core/GUID.h"

#include <cstdint>
#include <vector>

#include <memory>

#include <unordered_map>
#include <unordered_set>

#include <glm/glm.hpp>
#include <physx/PxPhysicsAPI.h>

using namespace physx;

namespace BC
{

    using ShapeType = uint8_t;
    enum : ShapeType
    {
        ShapeType_Unknown,
        ShapeType_Box,
        ShapeType_Sphere,
        ShapeType_Capsule,
        ShapeType_ConvexMesh,
        ShapeType_HeightField,
        ShapeType_TriangleMesh
    };

    struct RigidDynamicProperties
    {
		float mass = 1.0f;
		float drag = 0.0f;
		float angular_drag = 0.05f;

		bool use_gravity = true;
		bool is_kinematic = false;
		bool automatic_centre_of_mass = true;
        glm::vec3 manual_com_position = { 0.0f, 0.0f, 0.0f };

		glm::bvec3 position_constraint = { false, false, false };
		glm::bvec3 rotation_constraint = { false, false, false };
    };

    class RigidDynamic
    {

    public:

        RigidDynamic() = default;
        RigidDynamic(const RigidDynamicProperties& properties) : m_Properties(properties) {}
        ~RigidDynamic()
        {
            Shutdown();
        }

        RigidDynamic(const RigidDynamic& other) = delete;
        RigidDynamic(RigidDynamic&& other) noexcept
        {
            m_Handle = other.m_Handle;                              other.m_Handle = nullptr;
            m_Properties = std::move(other.m_Properties);           other.m_Properties = {};
            m_DeferredForce = std::move(other.m_DeferredForce);     other.m_DeferredForce = {};
            m_DeferredTorque = std::move(other.m_DeferredTorque);   other.m_DeferredTorque = {};
        }

        RigidDynamic& operator=(const RigidDynamic& other) = delete;
        RigidDynamic& operator=(RigidDynamic&& other) noexcept
        {
            if (this == &other)
                return *this;

            m_Handle = other.m_Handle;                              other.m_Handle = nullptr;
            m_Properties = std::move(other.m_Properties);           other.m_Properties = {};
            m_DeferredForce = std::move(other.m_DeferredForce);     other.m_DeferredForce = {};
            m_DeferredTorque = std::move(other.m_DeferredTorque);   other.m_DeferredTorque = {};

            return *this;
        }

        bool IsValid() const { return m_Handle != nullptr; }

        PxRigidDynamic* GetHandle() { return m_Handle; }

        void Init(const Entity& entity);
        void Shutdown();

        void AddShapeReference(Entity entity, ShapeType shape_type);

        const RigidDynamicProperties& GetProperties() const { return m_Properties; }
        void SetProperties(const RigidDynamicProperties& properties) { m_Properties = properties; }

        float GetMass() const { return m_Properties.mass; }
        void SetMass(float mass);

        float GetDrag() const { return m_Properties.drag; }
        void SetDrag(float drag);

        float GetAngularDrag() const { return m_Properties.angular_drag; }
        void SetAngularDrag(float angular_drag);

        bool IsUsingAutoCentreMass() const { return m_Properties.automatic_centre_of_mass; }
        void SetUsingAutoCentreMass(bool automatic_centre_of_mass);

        const glm::vec3& GetCentreOfMassIfNotAuto() const { return m_Properties.manual_com_position; }
        void SetCentreOfMassIfNotAuto(const glm::vec3& manual_com);

        bool IsUsingGravity() const { return m_Properties.use_gravity; }
        void SetUsingGravity(bool using_gravity);

        bool IsKinematic() const { return m_Properties.is_kinematic; }
        void SetKinematic(bool kinematic);

        const glm::bvec3& GetPositionConstraint() const { return m_Properties.position_constraint; }
        void SetPositionConstraint(const glm::bvec3& position_constraint);

        const glm::bvec3& GetRotationConstraint() const { return m_Properties.rotation_constraint; }
        void SetRotationConstraint(const glm::bvec3& rotation_constraint);

		void ApplyForce(const glm::vec3& force, PxForceMode::Enum force_mode = PxForceMode::eFORCE);
		void ApplyTorque(const glm::vec3& torque);
        void ApplyDeferredForces();

    private:

		/// @brief Handle for Rigid Dynamic Actor
		PxRigidDynamic* m_Handle = nullptr;

        /// @brief Properties of RigidDynamic
        RigidDynamicProperties m_Properties = {};

		struct DeferredForce 
        {
            DeferredForce(const glm::vec3& f, PxForceMode::Enum mode) : force(f), force_mode(mode) {}

			glm::vec3 force;
			PxForceMode::Enum force_mode;
		};

		struct DeferredTorque 
        {
            DeferredTorque(const glm::vec3& t) : torque(t) {}

			glm::vec3 torque;
		};

        /// @brief Vector of Deferred Forces Whilst PxScene is Simulating / Fetching Results
		std::vector<DeferredForce> m_DeferredForce = {};

        /// @brief Vector of Deferred Torque Whilst PxScene is Simulating / Fetching Results
		std::vector<DeferredTorque> m_DeferredTorque = {};
    };

    class PhysicsMaterial : public Asset
    {

    public:

        PhysicsMaterial();
        PhysicsMaterial(float dynamic_friction, float static_friction, float bounciness);
        ~PhysicsMaterial();

        PhysicsMaterial(const PhysicsMaterial& other);
        PhysicsMaterial(PhysicsMaterial&& other) noexcept;

        PhysicsMaterial& operator=(const PhysicsMaterial& other);
        PhysicsMaterial& operator=(PhysicsMaterial&& other) noexcept;

        AssetType GetType() const override { return AssetType::PhysicsMaterial; }

        bool IsValid() const { return m_Handle != nullptr; }

        void Init();
        void Shutdown();

		// ---- Getters ----

        PxMaterial* GetHandle() { return m_Handle; }

		float GetDynamicFriction() const { return m_DynamicFriction; }
		float GetStaticFriction() const { return m_StaticFriction; }
		float GetBounciness() const { return m_Bounciness; }

		// ---- Setters ----

		void SetDynamicFriction(float dynamic_friction) { m_DynamicFriction = dynamic_friction; }
		void SetStaticFriction(float static_friction) { m_StaticFriction = static_friction; }
		void SetBounciness(float bounciness) { m_Bounciness = bounciness; }

    private:

		PxMaterial* m_Handle = nullptr;

		float m_DynamicFriction = 0.6f;
		float m_StaticFriction = 0.6f;
		float m_Bounciness = 0.0f;
    };

    class PhysicsShape
    {

    public:
    
        virtual ~PhysicsShape() = default;

        virtual void Init(const Entity& entity) = 0;
        virtual void Shutdown() = 0;

        PxShape* GetHandle() { return m_Handle; }
        ShapeType GetType() const { return m_Type; }

        std::weak_ptr<RigidDynamic> GetStandaloneRigidDynamic() { return m_LocalRigidDynamic; }

        void UpdateRigidDynamicEntity(Entity& entity);
        void UpdateRigidDynamicEntity(RigidDynamic& rigid);
        void CreateLocalRigidDynamic(Entity& entity);
        void DestroyLocalRigidDynamic();

        bool IsValid() const { return m_Handle != nullptr; }

        void SetMaterialAssetHandle(AssetHandle asset_handle) { m_PhysicsMaterialHandle = asset_handle; }
        AssetHandle GetMaterialAssetHandle() const { return m_PhysicsMaterialHandle; }
        const PhysicsMaterial& GetMaterialInstance() const { return m_PhysicsMaterialInstance; }

        bool IsTrigger() const { return m_IsTrigger; }
        void SetIsTrigger(bool is_trigger)
        {
            m_IsTrigger = is_trigger;
            if (!m_Handle)
                return;

            m_Handle->setFlag(PxShapeFlag::eSIMULATION_SHAPE, !m_IsTrigger);
            m_Handle->setFlag(PxShapeFlag::eTRIGGER_SHAPE, m_IsTrigger);
        }

        const glm::vec3& GetCentre() const { return m_Centre; }
        void SetCentre(const glm::vec3& centre);

    protected:

        /// @brief Centre of the Shape
        glm::vec3 m_Centre = { 0.0f, 0.0f, 0.0f };

        /// @brief Handle for Shape
        PxShape* m_Handle = nullptr;

        /// @brief Type of the Shape
        ShapeType m_Type = ShapeType_Unknown;
        
        /// @brief The RigidDynamic if this collider is not attached to another Rigidbody owned by a RigidbodyComponent on an Entity 
        std::shared_ptr<RigidDynamic> m_LocalRigidDynamic = nullptr;

        /// @brief Physics Material Asset
        AssetHandle m_PhysicsMaterialHandle = NULL_GUID;

        /// @brief Physics Material Instance
        PhysicsMaterial m_PhysicsMaterialInstance = { 0.6f, 0.6f, 0.0f };

        bool m_IsTrigger = false;
    };

    class BoxPhysicsShape : public PhysicsShape
    {

    public:

        BoxPhysicsShape() { m_Type = ShapeType_Box; }
        BoxPhysicsShape(const glm::vec3& centre, const glm::vec3& half_extent) : m_HalfExtent(half_extent) { m_Centre = centre; m_Type = ShapeType_Box; }
        ~BoxPhysicsShape()
        {
            Shutdown();
        }

        BoxPhysicsShape(const BoxPhysicsShape& other) = delete;
        BoxPhysicsShape(BoxPhysicsShape&& other) noexcept
        {
            m_Handle = other.m_Handle;                                      other.m_Handle = nullptr;
            m_Type = other.m_Type;                                          other.m_Type = ShapeType_Unknown;
            m_LocalRigidDynamic = std::move(other.m_LocalRigidDynamic);     other.m_LocalRigidDynamic = nullptr;

            m_Centre = other.m_Centre;            other.m_Centre = { 0.0f, 0.0f, 0.0f };
            m_HalfExtent = other.m_HalfExtent;    other.m_HalfExtent = { 0.5f, 0.5f, 0.5f };
        }

        BoxPhysicsShape& operator=(const BoxPhysicsShape& other) = delete;
        BoxPhysicsShape& operator=(BoxPhysicsShape&& other) noexcept
        {
            if (this == &other)
                return *this;

            m_Handle = other.m_Handle;                                      other.m_Handle = nullptr;
            m_Type = other.m_Type;                                          other.m_Type = ShapeType_Unknown;
            m_LocalRigidDynamic = std::move(other.m_LocalRigidDynamic);     other.m_LocalRigidDynamic = nullptr;

            m_Centre = other.m_Centre;            other.m_Centre = { 0.0f, 0.0f, 0.0f };
            m_HalfExtent = other.m_HalfExtent;    other.m_HalfExtent = { 0.5f, 0.5f, 0.5f };

            return *this;
        }

        void Init(const Entity& entity) override;
        void Shutdown() override;
        
        const glm::vec3& GetHalfExtent() const { return m_HalfExtent; }
        void SetHalfExtent(const glm::vec3& half_extent);

        void UpdateBoxShape(const glm::vec3& final_extent);

    private:

        glm::vec3 m_HalfExtent = { 0.5f, 0.5f, 0.5f };
    
    };

    class SpherePhysicsShape : public PhysicsShape
    {

    public:

        SpherePhysicsShape() { m_Type = ShapeType_Sphere; }
        SpherePhysicsShape(const glm::vec3& centre, float radius) : m_Radius(radius) { m_Centre = centre; m_Type = ShapeType_Sphere; }
        ~SpherePhysicsShape()
        {
            Shutdown();
        }

        SpherePhysicsShape(const SpherePhysicsShape& other) = delete;
        SpherePhysicsShape(SpherePhysicsShape&& other) noexcept
        {
            m_Handle = other.m_Handle;                                      other.m_Handle = nullptr;
            m_Type = other.m_Type;                                          other.m_Type = ShapeType_Unknown;
            m_LocalRigidDynamic = std::move(other.m_LocalRigidDynamic);     other.m_LocalRigidDynamic = nullptr;

            m_Centre = other.m_Centre;  other.m_Centre = { 0.0f, 0.0f, 0.0f };
            m_Radius = other.m_Radius;  other.m_Radius = 0.5f;
        }

        SpherePhysicsShape& operator=(const SpherePhysicsShape& other) = delete;
        SpherePhysicsShape& operator=(SpherePhysicsShape&& other) noexcept
        {
            if (this == &other)
                return *this;

            m_Handle = other.m_Handle;                                      other.m_Handle = nullptr;
            m_Type = other.m_Type;                                          other.m_Type = ShapeType_Unknown;
            m_LocalRigidDynamic = std::move(other.m_LocalRigidDynamic);     other.m_LocalRigidDynamic = nullptr;

            m_Centre = other.m_Centre;  other.m_Centre = { 0.0f, 0.0f, 0.0f };
            m_Radius = other.m_Radius;  other.m_Radius = 0.5f;

            return *this;
        }

        void Init(const Entity& entity) override;
        void Shutdown() override;
        
        float GetRadius() const { return m_Radius; }
        void SetRadius(float radius);

        void UpdateSphereShape(float final_radius);

    private:
    
        float m_Radius = 0.5f;
    
    };

    class CapsulePhysicsShape : public PhysicsShape
    {

    public:

        CapsulePhysicsShape() { m_Type = ShapeType_Capsule; }
        CapsulePhysicsShape(const glm::vec3& centre, float radius, float half_height) : m_Radius(radius), m_HalfHeight(m_HalfHeight) { m_Centre = centre; m_Type = ShapeType_Capsule; }
        ~CapsulePhysicsShape()
        {
            Shutdown();
        }

        CapsulePhysicsShape(const CapsulePhysicsShape& other) = delete;
        CapsulePhysicsShape(CapsulePhysicsShape&& other) noexcept
        {
            m_Handle = other.m_Handle;                                      other.m_Handle = nullptr;
            m_Type = other.m_Type;                                          other.m_Type = ShapeType_Unknown;
            m_LocalRigidDynamic = std::move(other.m_LocalRigidDynamic);     other.m_LocalRigidDynamic = nullptr;

            m_Centre = other.m_Centre;  other.m_Centre = { 0.0f, 0.0f, 0.0f };
            m_Radius = other.m_Radius;  other.m_Radius = 0.5f;
            m_Radius = other.m_HalfHeight;  other.m_HalfHeight = 0.5f;
        }

        CapsulePhysicsShape& operator=(const CapsulePhysicsShape& other) = delete;
        CapsulePhysicsShape& operator=(CapsulePhysicsShape&& other) noexcept
        {
            if (this == &other)
                return *this;

            m_Handle = other.m_Handle;                                      other.m_Handle = nullptr;
            m_Type = other.m_Type;                                          other.m_Type = ShapeType_Unknown;
            m_LocalRigidDynamic = std::move(other.m_LocalRigidDynamic);     other.m_LocalRigidDynamic = nullptr;

            m_Centre = other.m_Centre;  other.m_Centre = { 0.0f, 0.0f, 0.0f };
            m_Radius = other.m_Radius;  other.m_Radius = 0.5f;

            return *this;
        }

        void Init(const Entity& entity) override;
        void Shutdown() override;
        
        float GetRadius() const { return m_Radius; }
        void SetRadius(float radius);
        
        float GetHalfHeight() const { return m_HalfHeight; }
        void SetHalfHeight(float half_height);

        void UpdateCapsuleShape(float final_radius, float final_half_height);

    private:
    
        float m_Radius = 0.5f;
        float m_HalfHeight = 0.5f;

    };

    class ConvexMeshPhysicsShape : public PhysicsShape
    {

    public:

        ConvexMeshPhysicsShape() { m_Type = ShapeType_ConvexMesh; }
        ~ConvexMeshPhysicsShape()
        {
            Shutdown();
        }

        ConvexMeshPhysicsShape(const ConvexMeshPhysicsShape& other) = delete;
        ConvexMeshPhysicsShape(ConvexMeshPhysicsShape&& other) noexcept
        {
            m_Handle = other.m_Handle;                                      other.m_Handle = nullptr;
            m_Type = other.m_Type;                                          other.m_Type = ShapeType_Unknown;
            m_LocalRigidDynamic = std::move(other.m_LocalRigidDynamic);     other.m_LocalRigidDynamic = nullptr;

            m_ConvexMeshColliderShapeAsset = other.m_ConvexMeshColliderShapeAsset;  other.m_ConvexMeshColliderShapeAsset = NULL_GUID;
        }

        ConvexMeshPhysicsShape& operator=(const ConvexMeshPhysicsShape& other) = delete;
        ConvexMeshPhysicsShape& operator=(ConvexMeshPhysicsShape&& other) noexcept
        {
            if (this == &other)
                return *this;

            m_Handle = other.m_Handle;                                      other.m_Handle = nullptr;
            m_Type = other.m_Type;                                          other.m_Type = ShapeType_Unknown;
            m_LocalRigidDynamic = std::move(other.m_LocalRigidDynamic);     other.m_LocalRigidDynamic = nullptr;

            m_ConvexMeshColliderShapeAsset = other.m_ConvexMeshColliderShapeAsset;  other.m_ConvexMeshColliderShapeAsset = NULL_GUID;

            return *this;
        }

        void Init(const Entity& entity) override;
        void Shutdown() override;


    private:
    
        // TODO: Think about this
        AssetHandle m_ConvexMeshColliderShapeAsset = NULL_GUID;

    };

    class HeightFieldPhysicsShape : public PhysicsShape
    {

    public:

        HeightFieldPhysicsShape() { m_Type = ShapeType_HeightField; }
        ~HeightFieldPhysicsShape()
        {
            Shutdown();
        }

        HeightFieldPhysicsShape(const HeightFieldPhysicsShape& other) = delete;
        HeightFieldPhysicsShape(HeightFieldPhysicsShape&& other) noexcept
        {
            m_Handle = other.m_Handle;                                      other.m_Handle = nullptr;
            m_Type = other.m_Type;                                          other.m_Type = ShapeType_Unknown;
            m_LocalRigidDynamic = std::move(other.m_LocalRigidDynamic);     other.m_LocalRigidDynamic = nullptr;

            m_ConvexMeshColliderShapeAsset = other.m_ConvexMeshColliderShapeAsset;  other.m_ConvexMeshColliderShapeAsset = NULL_GUID;
        }

        HeightFieldPhysicsShape& operator=(const HeightFieldPhysicsShape& other) = delete;
        HeightFieldPhysicsShape& operator=(HeightFieldPhysicsShape&& other) noexcept
        {
            if (this == &other)
                return *this;

            m_Handle = other.m_Handle;                                      other.m_Handle = nullptr;
            m_Type = other.m_Type;                                          other.m_Type = ShapeType_Unknown;
            m_LocalRigidDynamic = std::move(other.m_LocalRigidDynamic);     other.m_LocalRigidDynamic = nullptr;

            m_ConvexMeshColliderShapeAsset = other.m_ConvexMeshColliderShapeAsset;  other.m_ConvexMeshColliderShapeAsset = NULL_GUID;

            return *this;
        }

        void Init(const Entity& entity) override;
        void Shutdown() override;

    private:
    
        // TODO: Think about this
        AssetHandle m_ConvexMeshColliderShapeAsset = NULL_GUID;    

    };

    class TriangleMeshPhysicsShape : public PhysicsShape
    {

    public:

        TriangleMeshPhysicsShape() { m_Type = ShapeType_TriangleMesh; }
        ~TriangleMeshPhysicsShape()
        {
            Shutdown();
        }

        TriangleMeshPhysicsShape(const TriangleMeshPhysicsShape& other) = delete;
        TriangleMeshPhysicsShape(TriangleMeshPhysicsShape&& other) noexcept
        {
            m_Handle = other.m_Handle;                                      other.m_Handle = nullptr;
            m_Type = other.m_Type;                                          other.m_Type = ShapeType_Unknown;
            m_LocalRigidDynamic = std::move(other.m_LocalRigidDynamic);     other.m_LocalRigidDynamic = nullptr;

            m_ConvexMeshColliderShapeAsset = other.m_ConvexMeshColliderShapeAsset;  other.m_ConvexMeshColliderShapeAsset = NULL_GUID;
        }

        TriangleMeshPhysicsShape& operator=(const TriangleMeshPhysicsShape& other) = delete;
        TriangleMeshPhysicsShape& operator=(TriangleMeshPhysicsShape&& other) noexcept
        {
            if (this == &other)
                return *this;

            m_Handle = other.m_Handle;                                      other.m_Handle = nullptr;
            m_Type = other.m_Type;                                          other.m_Type = ShapeType_Unknown;
            m_LocalRigidDynamic = std::move(other.m_LocalRigidDynamic);     other.m_LocalRigidDynamic = nullptr;

            m_ConvexMeshColliderShapeAsset = other.m_ConvexMeshColliderShapeAsset;  other.m_ConvexMeshColliderShapeAsset = NULL_GUID;

            return *this;
        }

        void Init(const Entity& entity) override;
        void Shutdown() override;


    private:
    
        // TODO: Think about this
        AssetHandle m_ConvexMeshColliderShapeAsset = NULL_GUID;    

    };

}