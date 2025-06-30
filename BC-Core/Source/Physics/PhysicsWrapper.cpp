#include "BC_PCH.h"
#include "PhysicsWrapper.h"

#include "Asset/AssetManagerAPI.h"

namespace BC
{

#pragma region Rigidbody

    void RigidDynamic::Init(const Entity& entity)
    {
        if (!entity)
            return;

		glm::vec3 position = entity.GetComponent<TransformComponent>().GetGlobalPosition();
		glm::quat orientation = entity.GetComponent<TransformComponent>().GetGlobalOrientation();

        PxTransform transform = { position.x, position.y, position.z, PxQuat(orientation.x, orientation.y, orientation.z, orientation.w) };

        m_Handle = PxGetPhysics().createRigidDynamic(transform);

        SetMass(m_Properties.mass);
        SetDrag(m_Properties.drag);
        SetAngularDrag(m_Properties.angular_drag);
        SetUsingAutoCentreMass(m_Properties.automatic_centre_of_mass);
        if (!m_Properties.automatic_centre_of_mass)
        {
            SetCentreOfMassIfNotAuto(m_Properties.manual_com_position);
        }
        SetUsingGravity(m_Properties.use_gravity);
        SetKinematic(m_Properties.is_kinematic);
        SetPositionConstraint(m_Properties.position_constraint);
        SetRotationConstraint(m_Properties.rotation_constraint);
    }

    void RigidDynamic::Shutdown()
    {
        if (m_Handle) 
        {
            // Detach all attached shapes & alert Physics System of Colliders
            // that need rehoming
            PxU32 shape_count = m_Handle->getNbShapes();
            std::vector<PxShape*> shapes(shape_count);
            m_Handle->getShapes(shapes.data(), shape_count);

            auto project = Application::GetProject();
            if (project && project->GetSceneManager() && project->GetSceneManager()->GetPhysicsSystem())
            {
                auto physics_system = project->GetSceneManager()->GetPhysicsSystem();
                for (PxU32 i = 0; i < shape_count; ++i)
                {
                    PxShape* shape = shapes[i];
                    if (!shape)
                        continue;
                    
                    m_Handle->detachShape(*shape);

                    GUID shape_entity_guid = static_cast<GUID>(reinterpret_cast<uintptr_t>(shape->userData));
                    Entity shape_entity = Application::GetProject()->GetSceneManager()->GetEntity(shape_entity_guid);

                    ShapeType type;
                    switch (shape->getGeometry().getType())
                    {
                        case PxGeometryType::eBOX:              type = ShapeType_Box;           break;
                        case PxGeometryType::eSPHERE:           type = ShapeType_Sphere;        break;
                        case PxGeometryType::eCAPSULE:          type = ShapeType_Capsule;       break;
                        case PxGeometryType::eCONVEXMESH:       type = ShapeType_ConvexMesh;    break;
                        case PxGeometryType::eHEIGHTFIELD:      type = ShapeType_HeightField;   break;
                        case PxGeometryType::eTRIANGLEMESH:     type = ShapeType_TriangleMesh;  break;
                        default:                                type = ShapeType_Unknown;       break;
                    }
                    
                    physics_system->RegisterShape(shape_entity, shape, type);
                }
                physics_system->GetScene()->removeActor(*m_Handle);
            }
            
            m_Handle->release();
        }
        m_Handle = nullptr;
    }

    void RigidDynamic::AddShapeReference(Entity entity, ShapeType shape_type)
    {
        if (!m_Handle || !entity)
            return;
        
        GUID this_entity_guid = static_cast<GUID>(reinterpret_cast<uintptr_t>(m_Handle->userData));
        Entity this_entity = Application::GetProject()->GetSceneManager()->GetEntity(this_entity_guid);
        switch (shape_type)
        {
            case ShapeType_Box:
            {
                if (auto component = entity.TryGetComponent<BoxColliderComponent>(); component)
                {
                    component->GetShape()->UpdateRigidDynamicEntity(*this);
                }
                break;
            }
            case ShapeType_Sphere:
            {
                if (auto component = entity.TryGetComponent<SphereColliderComponent>(); component)
                {
                    component->GetShape()->UpdateRigidDynamicEntity(*this);
                }
                break;
            }
            case ShapeType_Capsule:
            {
                if (auto component = entity.TryGetComponent<CapsuleColliderComponent>(); component)
                {
                    component->GetShape()->UpdateRigidDynamicEntity(*this);
                }
                break;
            }
            case ShapeType_ConvexMesh:
            {
                if (auto component = entity.TryGetComponent<ConvexMeshColliderComponent>(); component)
                {
                    component->GetShape()->UpdateRigidDynamicEntity(*this);
                }
                break;
            }
            case ShapeType_HeightField:
            {
                if (auto component = entity.TryGetComponent<HeightFieldColliderComponent>(); component)
                {
                    component->GetShape()->UpdateRigidDynamicEntity(*this);
                }
                break;
            }
            case ShapeType_TriangleMesh:
            {
                if (auto component = entity.TryGetComponent<TriangleMeshColliderComponent>(); component)
                {
                    component->GetShape()->UpdateRigidDynamicEntity(*this);
                }
                break;
            }
        }

        SetUsingAutoCentreMass(m_Properties.automatic_centre_of_mass); // Ensure centre of mass updated
    }

    void RigidDynamic::SetMass(float mass)
    {
        m_Properties.mass = mass;
        if (!m_Handle)
            return;
        
        m_Handle->setMass(mass);
    }

    void RigidDynamic::SetDrag(float drag)
    {
        m_Properties.drag = drag;
        if (!m_Handle)
            return;
        
        m_Handle->setLinearDamping(drag);
    }

    void RigidDynamic::SetAngularDrag(float angular_drag)
    {
        m_Properties.angular_drag = angular_drag;
        if (!m_Handle)
            return;
        
        m_Handle->setAngularDamping(angular_drag);
    }

    void RigidDynamic::SetUsingAutoCentreMass(bool automatic_centre_of_mass)
    {
        m_Properties.automatic_centre_of_mass = automatic_centre_of_mass;
        if (!m_Handle)
            return;

        PxRigidBodyExt::setMassAndUpdateInertia(*m_Handle, m_Properties.mass);
        if (automatic_centre_of_mass)
        {
            // Reset COM transform to identity so PhysX auto-computes it
            m_Handle->setCMassLocalPose(PxTransform(PxIdentity));
        }
        else
        {
            m_Handle->setCMassLocalPose(PxTransform(m_Properties.manual_com_position.x, m_Properties.manual_com_position.y, m_Properties.manual_com_position.z));
        }
        PxRigidBodyExt::setMassAndUpdateInertia(*m_Handle, m_Properties.mass);
    }

    void RigidDynamic::SetCentreOfMassIfNotAuto(const glm::vec3 &manual_com)
    {
        m_Properties.manual_com_position = manual_com;

        if (!m_Handle)
            return;

        if (!m_Properties.automatic_centre_of_mass)
        {
            m_Handle->setCMassLocalPose(PxTransform(manual_com.x, manual_com.y, manual_com.z));
            PxRigidBodyExt::setMassAndUpdateInertia(*m_Handle, m_Properties.mass);
        }
    }

    void RigidDynamic::SetUsingGravity(bool using_gravity)
    {
        m_Properties.use_gravity = using_gravity;
        if (!m_Handle)
            return;
        
        m_Handle->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, !using_gravity);
    }

    void RigidDynamic::SetKinematic(bool kinematic)
    {
        m_Properties.is_kinematic = kinematic;
        if (!m_Handle)
            return;
        
        m_Handle->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, kinematic);

        if (!kinematic && m_Handle->getScene())
            m_Handle->wakeUp();
    }

    void RigidDynamic::SetPositionConstraint(const glm::bvec3& position_constraint)
    {
        m_Properties.position_constraint = position_constraint;

        if (!m_Handle)
            return;

        PxRigidDynamicLockFlags lock_flags = m_Handle->getRigidDynamicLockFlags();
        lock_flags.clear(PxRigidDynamicLockFlag::eLOCK_LINEAR_X);
        lock_flags.clear(PxRigidDynamicLockFlag::eLOCK_LINEAR_Y);
        lock_flags.clear(PxRigidDynamicLockFlag::eLOCK_LINEAR_Z);

        if (position_constraint.x)
            lock_flags |= PxRigidDynamicLockFlag::eLOCK_LINEAR_X;
        if (position_constraint.y)
            lock_flags |= PxRigidDynamicLockFlag::eLOCK_LINEAR_Y;
        if (position_constraint.z)
            lock_flags |= PxRigidDynamicLockFlag::eLOCK_LINEAR_Z;

        m_Handle->setRigidDynamicLockFlags(lock_flags);

        if (m_Handle->getScene())
            m_Handle->wakeUp();
    }

    void RigidDynamic::SetRotationConstraint(const glm::bvec3& rotation_constraint)
    {
        m_Properties.rotation_constraint = rotation_constraint;

        if (!m_Handle)
            return;

        PxRigidDynamicLockFlags lock_flags = m_Handle->getRigidDynamicLockFlags();
        lock_flags.clear(PxRigidDynamicLockFlag::eLOCK_ANGULAR_X);
        lock_flags.clear(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y);
        lock_flags.clear(PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z);

        if (rotation_constraint.x)
            lock_flags |= PxRigidDynamicLockFlag::eLOCK_ANGULAR_X;
        if (rotation_constraint.y)
            lock_flags |= PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y;
        if (rotation_constraint.z)
            lock_flags |= PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z;

        m_Handle->setRigidDynamicLockFlags(lock_flags);
       
        if (m_Handle->getScene())
            m_Handle->wakeUp();
    }

    void RigidDynamic::ApplyForce(const glm::vec3& force, PxForceMode::Enum force_mode)
    {
        if (!m_Handle)
            return;

        auto physics_ref = Application::GetProject()->GetSceneManager()->GetPhysicsSystem();
        if (!physics_ref)
            return;

        if (physics_ref->IsPhysicsSimulating())
            m_DeferredForce.emplace_back(force, force_mode);
        else
            m_Handle->addForce(PxVec3(force.x, force.y, force.z), force_mode);
    }

    void RigidDynamic::ApplyTorque(const glm::vec3& torque)
    {
        if (!m_Handle)
            return;

        auto physics_ref = Application::GetProject()->GetSceneManager()->GetPhysicsSystem();
        if (!physics_ref)
            return;

        if (physics_ref->IsPhysicsSimulating())
            m_DeferredTorque.emplace_back(torque);
        else
            m_Handle->addTorque(PxVec3(torque.x, torque.y, torque.z));
    }

    void RigidDynamic::ApplyDeferredForces()
    {
        if (m_DeferredForce.empty() && m_DeferredTorque.empty())
            return;
        
        for (const auto& force : m_DeferredForce)
        {
            ApplyForce(force.force, force.force_mode);
        }
        
        for (const auto& torque : m_DeferredTorque)
        {
            ApplyTorque(torque.torque);
        }
    }

#pragma endregion

#pragma region Shapes

    void PhysicsShape::UpdateRigidDynamicEntity(Entity& entity)
    {
        if (!m_Handle)
            return;

        auto rigidbody_component = entity.TryGetComponent<RigidbodyComponent>();
        if (!rigidbody_component)
        {
            GUID this_entity_guid = static_cast<GUID>(reinterpret_cast<uintptr_t>(m_Handle->userData));
            Entity this_entity = Application::GetProject()->GetSceneManager()->GetEntity(this_entity_guid);
            if (entity.GetGUID() != this_entity_guid || !entity)
            {
                BC_CORE_WARN("PhysicsShape::UpdateRigidDynamicEntity: Entity missing RigidbodyComponent.");
                return;
            }

            CreateLocalRigidDynamic(this_entity);
            return;
        }

        auto rigid = rigidbody_component->GetRigid();
        if (!rigid || !rigid->GetHandle())
        {
            BC_CORE_WARN("PhysicsShape::UpdateRigidDynamicEntity: Invalid or uninitialized RigidDynamic.");
            return;
        }
        UpdateRigidDynamicEntity(*rigid);
    }

    void PhysicsShape::UpdateRigidDynamicEntity(RigidDynamic& rigid)
    {
        if (!m_Handle)
            return;

        GUID shape_guid = static_cast<GUID>(reinterpret_cast<uintptr_t>(m_Handle->userData));
        Entity shape_entity = Application::GetProject()->GetSceneManager()->GetEntity(shape_guid);

        PxActor* current_rigid_handle = m_Handle->getActor();
        if (current_rigid_handle && current_rigid_handle->is<PxRigidDynamic>())
        {
            PxRigidDynamic* current_rigid_actor = reinterpret_cast<PxRigidDynamic*>(current_rigid_handle);

            current_rigid_actor->detachShape(*m_Handle);
            
            GUID old_rigid_guid = static_cast<GUID>(reinterpret_cast<uintptr_t>(current_rigid_actor->userData));
            Entity old_rigid_entity = Application::GetProject()->GetSceneManager()->GetEntity(old_rigid_guid);
            if (old_rigid_entity && old_rigid_guid != shape_guid)
            {
                auto old_rigid_component = old_rigid_entity.TryGetComponent<RigidbodyComponent>();
                if (old_rigid_component)
                {
                    // Reset centre of mass on old rigidbody
                    old_rigid_component->GetRigid()->SetUsingAutoCentreMass(old_rigid_component->GetRigid()->IsUsingAutoCentreMass());
                }
            }
        }

        GUID rigid_guid = static_cast<GUID>(reinterpret_cast<uintptr_t>(rigid.GetHandle()->userData));
        Entity rigid_entity = Application::GetProject()->GetSceneManager()->GetEntity(rigid_guid);
        if (!rigid_entity || !shape_entity)
            return;

        glm::mat4 shape_global_matrix = shape_entity.GetTransform().GetGlobalMatrix();
        glm::mat4 rigid_global_matrix = rigid_entity.GetTransform().GetGlobalMatrix();

        // calculate correct relative PxTransform. If shape type is capsule, do custom logic to ensure XUP is transformed to YUP
        glm::mat4 local_matrix = glm::inverse(rigid_global_matrix) * shape_global_matrix;

        // Extract position and rotation
        glm::vec3 local_position = TransformComponent::GetPositionFromMatrix(local_matrix);
        glm::quat local_rotation = TransformComponent::GetOrientationFromMatrix(local_matrix);

        // Convert to PhysX
        PxVec3 px_pos(local_position.x, local_position.y, local_position.z);
        PxQuat px_rot(local_rotation.x, local_rotation.y, local_rotation.z, local_rotation.w);

        // Handle capsule up axis correction if needed
        if (GetType() == ShapeType_Capsule)
        {
            // Rotate capsule from +X (PhysX default) to +Y (engine)
            PxQuat xup_to_yup = PxQuat(PxPiDivTwo, PxVec3(0, 0, 1));
            px_rot = px_rot * xup_to_yup;
            px_rot.normalize();
        }

        m_Handle->setLocalPose(PxTransform(px_pos, px_rot));

        // Now attach the shape with correct local pose
        rigid.GetHandle()->attachShape(*m_Handle);
    }

    void PhysicsShape::CreateLocalRigidDynamic(Entity& entity)
    {
        if (!entity)
            return;

        DestroyLocalRigidDynamic();

        RigidDynamicProperties properties{};
        properties.use_gravity = false;
        properties.is_kinematic = true;
        properties.position_constraint = { true, true, true };
        properties.rotation_constraint = { true, true, true };

        m_LocalRigidDynamic = std::make_unique<RigidDynamic>(properties);
        m_LocalRigidDynamic->Init(entity);
        m_LocalRigidDynamic->GetHandle()->userData = reinterpret_cast<void*>(static_cast<uintptr_t>(entity.GetGUID()));

        if (auto physics_system = Application::GetProject()->GetSceneManager()->GetPhysicsSystem(); physics_system && m_LocalRigidDynamic->GetHandle())
        {
            physics_system->GetScene()->addActor(*m_LocalRigidDynamic->GetHandle());
        }
    }

    void PhysicsShape::DestroyLocalRigidDynamic()
    {
        if (m_LocalRigidDynamic)
        {
            m_LocalRigidDynamic.reset();
            m_LocalRigidDynamic = nullptr;
        }
    }

    void PhysicsShape::SetCentre(const glm::vec3 &centre)
    {
        m_Centre = centre;
        if (!m_Handle)
            return;

        PxTransform local_pose = m_Handle->getLocalPose();
        local_pose.p = PxVec3(centre.x, centre.y, centre.z);

        if (GetType() == ShapeType_Capsule)
        {
            // Capsules are aligned along +X by default; rotate them +90° around Z to point up (Y+)
            GUID entity_guid = static_cast<GUID>(reinterpret_cast<uintptr_t>(m_Handle->userData));
            Entity entity = Application::GetProject()->GetSceneManager()->GetEntity(entity_guid);
            if (entity)
            {
                glm::quat entity_orientation = TransformComponent::GetOrientationFromMatrix(entity.GetTransform().GetGlobalMatrix());

                // Rotate capsule from +X to +Y (PhysX capsule default to +X)
                PxQuat rotate_x_to_y = PxQuat(PxPiDivTwo, PxVec3(0, 0, 1));

                // Apply entity orientation after rotating the capsule up
                PxQuat final_orientation = PxQuat(entity_orientation.x, entity_orientation.y, entity_orientation.z, entity_orientation.w) * rotate_x_to_y;

                local_pose.q = final_orientation;
                m_Handle->setLocalPose(local_pose);
            }
            else
            {
                local_pose.q = PxQuat(PxPiDivTwo, PxVec3(0, 0, 1));
                m_Handle->setLocalPose(local_pose);
            }
        }
        else
        {
            local_pose.q = PxQuat(PxIdentity);
            m_Handle->setLocalPose(local_pose);
        }
    }

    void BoxPhysicsShape::Init(const Entity& entity)
    {
        if (m_Handle)
            return;
            
        if (AssetManager::IsAssetHandleValid(m_PhysicsMaterialHandle))
        {
            if (auto physics_material = AssetManager::GetAsset<PhysicsMaterial>(m_PhysicsMaterialHandle); physics_material)
                m_PhysicsMaterialInstance = *physics_material;
        }
        else
        {
            m_PhysicsMaterialInstance = {};
        }
        m_PhysicsMaterialInstance.Init();

        glm::vec3 final_extent = glm::abs(m_HalfExtent);
        if (entity)
        {
            final_extent *= glm::abs(entity.GetTransform().GetGlobalScale());
        }

        m_Handle = PxGetPhysics().createShape(PxBoxGeometry(final_extent.x, final_extent.y, final_extent.z), *m_PhysicsMaterialInstance.GetHandle(), true);
    }

    void BoxPhysicsShape::SetHalfExtent(const glm::vec3& half_extent)
    {
        m_HalfExtent = glm::abs(half_extent);
        if (!m_Handle)
            return;

        GUID entity_guid = static_cast<GUID>(reinterpret_cast<uintptr_t>(m_Handle->userData));
        Entity entity = Application::GetProject()->GetSceneManager()->GetEntity(entity_guid);
        if (!entity)
            return;

        glm::vec3 final_extent = glm::abs(m_HalfExtent);
        final_extent *= glm::abs(entity.GetTransform().GetGlobalScale());
        UpdateBoxShape(final_extent);
    }

    void BoxPhysicsShape::UpdateBoxShape(const glm::vec3& final_extent)
    {
        if (!m_Handle)
            return;
            
        PxBoxGeometry new_geom(final_extent.x, final_extent.y, final_extent.z);
        m_Handle->setGeometry(new_geom);
    }

    void BoxPhysicsShape::Shutdown()
    {
        m_PhysicsMaterialInstance.Shutdown();
        if (m_Handle && m_Handle->isReleasable())
        {
            PxActor* rigid_handle = m_Handle->getActor();
            if (rigid_handle && rigid_handle->is<PxRigidDynamic>())
                reinterpret_cast<PxRigidDynamic*>(rigid_handle)->detachShape(*m_Handle);
            m_Handle->release();
        }

        m_Handle = nullptr;
    }

    void SpherePhysicsShape::Init(const Entity& entity)
    {
        if (m_Handle)
            return;

        if (AssetManager::IsAssetHandleValid(m_PhysicsMaterialHandle))
        {
            if (auto physics_material = AssetManager::GetAsset<PhysicsMaterial>(m_PhysicsMaterialHandle); physics_material)
                m_PhysicsMaterialInstance = *physics_material;
        }
        else
        {
            m_PhysicsMaterialInstance = {};
        }
        m_PhysicsMaterialInstance.Init();

        float final_radius = glm::abs(m_Radius);
        if (entity)
        {
            final_radius *= glm::compMax(glm::abs(entity.GetTransform().GetGlobalScale()));
        }

        m_Handle = PxGetPhysics().createShape(PxSphereGeometry(final_radius), *m_PhysicsMaterialInstance.GetHandle(), true);
    }

    void SpherePhysicsShape::SetRadius(float radius)
        {
            m_Radius = glm::abs(radius);
            if (!m_Handle)
                return;

            GUID entity_guid = static_cast<GUID>(reinterpret_cast<uintptr_t>(m_Handle->userData));
            Entity entity = Application::GetProject()->GetSceneManager()->GetEntity(entity_guid);
            if (!entity)
                return;
        
            float final_radius = glm::abs(m_Radius);
            final_radius *= glm::compMax(glm::abs(entity.GetTransform().GetGlobalScale()));
            UpdateSphereShape(final_radius);
        }

    void SpherePhysicsShape::UpdateSphereShape(float final_radius)
    {
        if (!m_Handle)
            return;

        PxSphereGeometry new_geom(final_radius);
        m_Handle->setGeometry(new_geom);
    }

    void SpherePhysicsShape::Shutdown()
    {
        m_PhysicsMaterialInstance.Shutdown();
        if (m_Handle && m_Handle->isReleasable())
        {
            PxActor* rigid_handle = m_Handle->getActor();
            if (rigid_handle && rigid_handle->is<PxRigidDynamic>())
                reinterpret_cast<PxRigidDynamic*>(rigid_handle)->detachShape(*m_Handle);
            m_Handle->release();
        }

        m_Handle = nullptr;
    }

    void CapsulePhysicsShape::Init(const Entity& entity)
    {
        if (m_Handle)
            return;

        if (AssetManager::IsAssetHandleValid(m_PhysicsMaterialHandle))
        {
            if (auto physics_material = AssetManager::GetAsset<PhysicsMaterial>(m_PhysicsMaterialHandle); physics_material)
                m_PhysicsMaterialInstance = *physics_material;
        }
        else
        {
            m_PhysicsMaterialInstance = {};
        }
        m_PhysicsMaterialInstance.Init();

        float final_radius = glm::abs(m_Radius);
        float final_half_height = glm::abs(m_HalfHeight);
        if (entity)
        {
            glm::vec3 scale = glm::abs(entity.GetTransform().GetGlobalScale());
            final_radius *= glm::max(scale.x, scale.z);
            final_half_height *= scale.y;
        }

        m_Handle = PxGetPhysics().createShape(PxCapsuleGeometry(final_radius, final_half_height), *m_PhysicsMaterialInstance.GetHandle(), true);
    }

    void CapsulePhysicsShape::SetRadius(float radius)
    {
        m_Radius = glm::abs(radius);
        if (!m_Handle)
            return;

        GUID entity_guid = static_cast<GUID>(reinterpret_cast<uintptr_t>(m_Handle->userData));
        Entity entity = Application::GetProject()->GetSceneManager()->GetEntity(entity_guid);
        if (!entity)
            return;

        glm::vec3 scale = glm::abs(entity.GetTransform().GetGlobalScale());

        float final_radius = m_Radius * glm::max(scale.x, scale.z);
        float final_half_height = m_HalfHeight * scale.y;

        UpdateCapsuleShape(final_radius, final_half_height);
    }
    
    void CapsulePhysicsShape::SetHalfHeight(float half_height)
    {
        m_HalfHeight = glm::abs(half_height);
        if (!m_Handle)
            return;

        GUID entity_guid = static_cast<GUID>(reinterpret_cast<uintptr_t>(m_Handle->userData));
        Entity entity = Application::GetProject()->GetSceneManager()->GetEntity(entity_guid);
        if (!entity)
            return;

        glm::vec3 scale = glm::abs(entity.GetTransform().GetGlobalScale());

        float final_radius = m_Radius * glm::max(scale.x, scale.z);
        float final_half_height = m_HalfHeight * scale.y;

        UpdateCapsuleShape(final_radius, final_half_height);
    }

    void CapsulePhysicsShape::UpdateCapsuleShape(float final_radius, float final_half_height)
    {
        if (!m_Handle)
            return;

        PxCapsuleGeometry new_geom(final_radius, final_half_height);
        m_Handle->setGeometry(new_geom);
    }

    void CapsulePhysicsShape::Shutdown()
    {
        m_PhysicsMaterialInstance.Shutdown();
        if (m_Handle && m_Handle->isReleasable())
        {
            PxActor* rigid_handle = m_Handle->getActor();
            if (rigid_handle && rigid_handle->is<PxRigidDynamic>())
                reinterpret_cast<PxRigidDynamic*>(rigid_handle)->detachShape(*m_Handle);
            m_Handle->release();
        }

        m_Handle = nullptr;
    }

    void ConvexMeshPhysicsShape::Init(const Entity& entity)
    {
        // TODO:
    }

    void ConvexMeshPhysicsShape::Shutdown()
    {
        m_PhysicsMaterialInstance.Shutdown();
        if (m_Handle && m_Handle->isReleasable())
            m_Handle->release();
        m_Handle = nullptr;
    }

    void HeightFieldPhysicsShape::Init(const Entity& entity)
    {
        // TODO:
    }

    void HeightFieldPhysicsShape::Shutdown()
    {
        m_PhysicsMaterialInstance.Shutdown();
        if (m_Handle && m_Handle->isReleasable())
            m_Handle->release();
        m_Handle = nullptr;
    }

    void TriangleMeshPhysicsShape::Init(const Entity& entity)
    {
        // TODO:
    }

    void TriangleMeshPhysicsShape::Shutdown()
    {
        m_PhysicsMaterialInstance.Shutdown();
        if (m_Handle && m_Handle->isReleasable())
            m_Handle->release();
        m_Handle = nullptr;
    }

#pragma endregion

#pragma region Material

    PhysicsMaterial::PhysicsMaterial()
    {
    }

    PhysicsMaterial::PhysicsMaterial(float dynamic_friction, float static_friction, float bounciness) :
        m_DynamicFriction(dynamic_friction),
        m_StaticFriction(static_friction),
        m_Bounciness(bounciness)
    {
    }

    PhysicsMaterial::~PhysicsMaterial()
    {
    }

    PhysicsMaterial::PhysicsMaterial(const PhysicsMaterial& other)
    {
        m_DynamicFriction = other.m_DynamicFriction;
        m_StaticFriction = other.m_StaticFriction;
        m_Bounciness = other.m_Bounciness;
    }

    PhysicsMaterial::PhysicsMaterial(PhysicsMaterial&& other) noexcept
    {
        m_Handle = other.m_Handle;                      other.m_Handle = nullptr;
        m_DynamicFriction = other.m_DynamicFriction;    other.m_DynamicFriction = 0.6f;
        m_StaticFriction = other.m_StaticFriction;      other.m_StaticFriction = 0.6f;
        m_Bounciness = other.m_Bounciness;              other.m_Bounciness = 0.0f;
    }

    PhysicsMaterial& PhysicsMaterial::operator=(const PhysicsMaterial &other)
    {
        if (this == &other)
            return *this;

        m_DynamicFriction = other.m_DynamicFriction;
        m_StaticFriction = other.m_StaticFriction;
        m_Bounciness = other.m_Bounciness;

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
        if (m_Handle)
            m_Handle->release();
        m_Handle = nullptr;
    }

#pragma endregion

}