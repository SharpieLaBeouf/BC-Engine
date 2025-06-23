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
        ShapeType_Plane,
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

		bool automatic_centre_of_mass = true;
		bool use_gravity = true;
		bool is_kinematic = false;

		glm::bvec3 position_constraint = { false, false, false };
		glm::bvec3 rotation_constraint = { false, false, false };
    };

    class RigidDynamic
    {

    public:

        RigidDynamic() = default;
        RigidDynamic(const RigidDynamicProperties& properties) : m_Properties(properties) {}
        ~RigidDynamic();

        RigidDynamic(const RigidDynamic& other) = delete;
        RigidDynamic(RigidDynamic&& other) noexcept;

        RigidDynamic& operator=(const RigidDynamic& other) = delete;
        RigidDynamic& operator=(RigidDynamic&& other) noexcept;

        explicit operator bool() const { return m_Handle != nullptr; }

        PxRigidDynamic* GetHandle() { return m_Handle; }

        void Init();
        void Shutdown();

        void AddShapeReference(GUID entity_id, ShapeType shape_type);
        void RemoveShapeReference(GUID entity_id, ShapeType shape_type);

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

        bool IsUsingGravity() const { return m_Properties.use_gravity; }
        void SetUsingGravity(bool using_gravity);

        bool IsKinematic() const { return m_Properties.is_kinematic; }
        void SetKinematic(bool kinematic);

        const glm::bvec3& GetPositionConstraint() const { return m_Properties.position_constraint; }
        void SetPositionConstraint(bool position_constraint);

        const glm::bvec3& GetRotationConstraint() const { return m_Properties.rotation_constraint; }
        void SetRotationConstraint(bool rotation_constraint);

		void ApplyForce(const glm::vec3& force, PxForceMode::Enum force_mode = PxForceMode::eFORCE);
		void ApplyTorque(const glm::vec3& torque);

    private:

		/// @brief Handle for Rigid Dynamic Actor
		PxRigidDynamic* m_Handle = nullptr;

        /// @brief Attached Shapes
        std::unordered_map<GUID, std::unordered_set<ShapeType>> m_Shapes = {};

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

        explicit operator bool() const { return m_Handle != nullptr; }

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

        virtual void Init() = 0;
        virtual void Shutdown() = 0;

        PxShape* GetHandle() { return m_Handle; }
        ShapeType GetType() const { return m_Type; }

        GUID GetRigidDynamicEntity() const { return m_RigidEntityID; }
        std::weak_ptr<RigidDynamic> GetStandaloneRigidDynamic() { return m_StandAloneRigidDynamic; }

        explicit operator bool() const { return m_Handle != nullptr; }

    protected:

        /// @brief Handle for Shape
        PxShape* m_Handle = nullptr;

        /// @brief Type of the Shape
        ShapeType m_Type = ShapeType_Unknown;

        /// @brief The Entity GUID if this is tied to a Rigidbody owned by a RigidbodyComponent on an Entity
        GUID m_RigidEntityID = NULL_GUID;

        /// @brief The RigidDynamic if this collider is not attached to another Rigidbody owned by a RigidbodyComponent on an Entity 
        std::shared_ptr<RigidDynamic> m_StandAloneRigidDynamic = nullptr;
    };

    class PlanePhysicsShape : public PhysicsShape
    {

    public:

        PlanePhysicsShape();
        ~PlanePhysicsShape();

        PlanePhysicsShape(const PlanePhysicsShape& other);
        PlanePhysicsShape(PlanePhysicsShape&& other) noexcept;

        PlanePhysicsShape& operator=(const PlanePhysicsShape& other);
        PlanePhysicsShape& operator=(PlanePhysicsShape&& other) noexcept;

        void Init() override;
        void Shutdown() override;


    private:


    };

    class BoxPhysicsShape : public PhysicsShape
    {

    public:

        BoxPhysicsShape();
        ~BoxPhysicsShape();

        BoxPhysicsShape(const BoxPhysicsShape& other);
        BoxPhysicsShape(BoxPhysicsShape&& other) noexcept;

        BoxPhysicsShape& operator=(const BoxPhysicsShape& other);
        BoxPhysicsShape& operator=(BoxPhysicsShape&& other) noexcept;

        void Init() override;
        void Shutdown() override;


    private:
    

    };

    class SpherePhysicsShape : public PhysicsShape
    {

    public:

        SpherePhysicsShape();
        ~SpherePhysicsShape();

        SpherePhysicsShape(const SpherePhysicsShape& other);
        SpherePhysicsShape(SpherePhysicsShape&& other) noexcept;

        SpherePhysicsShape& operator=(const SpherePhysicsShape& other);
        SpherePhysicsShape& operator=(SpherePhysicsShape&& other) noexcept;

        void Init() override;
        void Shutdown() override;


    private:
    

    };

    class CapsulePhysicsShape : public PhysicsShape
    {

    public:

        CapsulePhysicsShape();
        ~CapsulePhysicsShape();

        CapsulePhysicsShape(const CapsulePhysicsShape& other);
        CapsulePhysicsShape(CapsulePhysicsShape&& other) noexcept;

        CapsulePhysicsShape& operator=(const CapsulePhysicsShape& other);
        CapsulePhysicsShape& operator=(CapsulePhysicsShape&& other) noexcept;

        void Init() override;
        void Shutdown() override;


    private:
    

    };

    class ConvexMeshPhysicsShape : public PhysicsShape
    {

    public:

        ConvexMeshPhysicsShape();
        ~ConvexMeshPhysicsShape();

        ConvexMeshPhysicsShape(const ConvexMeshPhysicsShape& other);
        ConvexMeshPhysicsShape(ConvexMeshPhysicsShape&& other) noexcept;

        ConvexMeshPhysicsShape& operator=(const ConvexMeshPhysicsShape& other);
        ConvexMeshPhysicsShape& operator=(ConvexMeshPhysicsShape&& other) noexcept;

        void Init() override;
        void Shutdown() override;


    private:
    

    };

    class HeightFieldPhysicsShape : public PhysicsShape
    {

    public:

        HeightFieldPhysicsShape();
        ~HeightFieldPhysicsShape();

        HeightFieldPhysicsShape(const HeightFieldPhysicsShape& other);
        HeightFieldPhysicsShape(HeightFieldPhysicsShape&& other) noexcept;

        HeightFieldPhysicsShape& operator=(const HeightFieldPhysicsShape& other);
        HeightFieldPhysicsShape& operator=(HeightFieldPhysicsShape&& other) noexcept;

        void Init() override;
        void Shutdown() override;


    private:
    

    };

    class TriangleMeshPhysicsShape : public PhysicsShape
    {

    public:

        TriangleMeshPhysicsShape();
        ~TriangleMeshPhysicsShape();

        TriangleMeshPhysicsShape(const TriangleMeshPhysicsShape& other);
        TriangleMeshPhysicsShape(TriangleMeshPhysicsShape&& other) noexcept;

        TriangleMeshPhysicsShape& operator=(const TriangleMeshPhysicsShape& other);
        TriangleMeshPhysicsShape& operator=(TriangleMeshPhysicsShape&& other) noexcept;

        void Init() override;
        void Shutdown() override;


    private:
    

    };

}