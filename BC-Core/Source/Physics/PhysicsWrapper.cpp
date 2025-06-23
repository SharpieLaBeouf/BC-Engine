#include "BC_PCH.h"
#include "PhysicsWrapper.h"

namespace BC
{

#pragma region Rigidbody

    RigidDynamic::~RigidDynamic()
    {
        Shutdown();
    }

    RigidDynamic::RigidDynamic(RigidDynamic&& other) noexcept
    {
        m_Handle = other.m_Handle;                              other.m_Handle = nullptr;
        m_Shapes = std::move(other.m_Shapes);                   other.m_Shapes = {};
        m_Properties = std::move(other.m_Properties);           other.m_Properties = {};
        m_DeferredForce = std::move(other.m_DeferredForce);     other.m_DeferredForce = {};
        m_DeferredTorque = std::move(other.m_DeferredTorque);   other.m_DeferredTorque = {};
    }

    RigidDynamic& RigidDynamic::operator=(RigidDynamic&& other) noexcept
    {
        if (this == &other)
            return *this;

        m_Handle = other.m_Handle;                              other.m_Handle = nullptr;
        m_Shapes = std::move(other.m_Shapes);                   other.m_Shapes = {};
        m_Properties = std::move(other.m_Properties);           other.m_Properties = {};
        m_DeferredForce = std::move(other.m_DeferredForce);     other.m_DeferredForce = {};
        m_DeferredTorque = std::move(other.m_DeferredTorque);   other.m_DeferredTorque = {};

        return *this;
    }

    void RigidDynamic::Init()
    {
    }

    void RigidDynamic::Shutdown()
    {
    }

    void RigidDynamic::AddShapeReference(GUID entity_id, ShapeType shape_type)
    {
    }

    void RigidDynamic::RemoveShapeReference(GUID entity_id, ShapeType shape_type)
    {
    }

    void RigidDynamic::SetMass(float mass)
    {
    }

    void RigidDynamic::SetDrag(float drag)
    {
    }

    void RigidDynamic::SetAngularDrag(float angular_drag)
    {
    }

    void RigidDynamic::SetUsingAutoCentreMass(bool automatic_centre_of_mass)
    {
    }

    void RigidDynamic::SetUsingGravity(bool using_gravity)
    {
    }

    void RigidDynamic::SetKinematic(bool kinematic)
    {
    }

    void RigidDynamic::SetPositionConstraint(bool position_constraint)
    {
    }

    void RigidDynamic::SetRotationConstraint(bool rotation_constraint)
    {
    }

    void RigidDynamic::ApplyForce(const glm::vec3& force, PxForceMode::Enum force_mode)
    {
        if (!m_Handle)
            return;

        if (Application::GetProject()->GetSceneManager()->IsPhysicsSimulating())
            m_DeferredForce.emplace_back(force, force_mode);
        else
            m_Handle->addForce(PxVec3(force.x, force.y, force.z), force_mode);
    }

    void RigidDynamic::ApplyTorque(const glm::vec3& torque)
    {
        if (!m_Handle)
            return;

        if (Application::GetProject()->GetSceneManager()->IsPhysicsSimulating())
            m_DeferredTorque.emplace_back(torque);
        else
            m_Handle->addTorque(PxVec3(torque.x, torque.y, torque.z));
    }

#pragma endregion

#pragma region Shapes

#pragma endregion

#pragma region Material

    PhysicsMaterial::PhysicsMaterial()
    {
        Init();
    }

    PhysicsMaterial::PhysicsMaterial(float dynamic_friction, float static_friction, float bounciness) :
        m_DynamicFriction(dynamic_friction),
        m_StaticFriction(static_friction),
        m_Bounciness(bounciness)
    {
        Init();
    }

    PhysicsMaterial::~PhysicsMaterial()
    {
        Shutdown();
    }

    PhysicsMaterial::PhysicsMaterial(const PhysicsMaterial& other)
    {
        m_DynamicFriction = other.m_DynamicFriction;
        m_StaticFriction = other.m_StaticFriction;
        m_Bounciness = other.m_Bounciness;

        Init();
    }

    PhysicsMaterial::PhysicsMaterial(PhysicsMaterial&& other) noexcept
    {
        m_Handle = other.m_Handle;                      other.m_Handle = nullptr;
        m_DynamicFriction = other.m_DynamicFriction;    other.m_DynamicFriction = 0.6f;
        m_StaticFriction = other.m_StaticFriction;      other.m_StaticFriction = 0.6f;
        m_Bounciness = other.m_Bounciness;              other.m_Bounciness = 0.0f;

        if (!m_Handle)
            Init();
    }

    PhysicsMaterial& PhysicsMaterial::operator=(const PhysicsMaterial &other)
    {
        if (this == &other)
            return *this;

        m_DynamicFriction = other.m_DynamicFriction;
        m_StaticFriction = other.m_StaticFriction;
        m_Bounciness = other.m_Bounciness;

        Init();

        return *this;
    }

    PhysicsMaterial& PhysicsMaterial::operator=(PhysicsMaterial &&other) noexcept
    {
        if (this == &other)
            return *this;

        m_Handle = other.m_Handle;                      other.m_Handle = nullptr;
        m_DynamicFriction = other.m_DynamicFriction;    other.m_DynamicFriction = 0.6f;
        m_StaticFriction = other.m_StaticFriction;      other.m_StaticFriction = 0.6f;
        m_Bounciness = other.m_Bounciness;              other.m_Bounciness = 0.0f;

        if (!m_Handle)
            Init();

        return *this;
    }

    void PhysicsMaterial::Init()
    {
        if (m_Handle)
        {
            m_Handle->setStaticFriction(m_StaticFriction);
            m_Handle->setDynamicFriction(m_DynamicFriction);
            m_Handle->setRestitution(m_Bounciness);
        }
        else
        {
            m_Handle = PxGetPhysics().createMaterial(m_StaticFriction, m_DynamicFriction, m_Bounciness);
        }
    }

    void PhysicsMaterial::Shutdown()
    {
        if (m_Handle && m_Handle->isReleasable())
            m_Handle->release();
    }

#pragma endregion


}