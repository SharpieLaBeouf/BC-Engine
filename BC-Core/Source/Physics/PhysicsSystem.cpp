#include "BC_PCH.h"
#include "PhysicsSystem.h"

namespace BC
{

#pragma region CollisionCallback

    void CollisionCallback::onContact(const PxContactPairHeader& pair_header, const PxContactPair* pairs, PxU32 pairs_count)
    {
    }

    void CollisionCallback::onTrigger(PxTriggerPair *, PxU32)
    {
    }

    void CollisionCallback::onConstraintBreak(PxConstraintInfo *, PxU32)
    {
    }

    void CollisionCallback::onAdvance(const PxRigidBody* const* body_buffer, const PxTransform *pose_buffer, const PxU32 count)
    {
    }

    void CollisionCallback::onWake(PxActor **, PxU32)
    {
    }

    void CollisionCallback::onSleep(PxActor **, PxU32)
    {
    }

#pragma endregion

#pragma region PhysicsSystem

	static PxFilterFlags CustomFilterShader
    (
		PxFilterObjectAttributes attributes0, PxFilterData filter_data0,
		PxFilterObjectAttributes attributes1, PxFilterData filter_data1,
		PxPairFlags& pair_flags, const void* constant_block, PxU32 constant_block_size
    )
	{
		pair_flags = PxPairFlag::eCONTACT_DEFAULT | PxPairFlag::eNOTIFY_TOUCH_FOUND | PxPairFlag::eNOTIFY_TOUCH_PERSISTS | PxPairFlag::eNOTIFY_TOUCH_LOST;
		return PxFilterFlag::eDEFAULT;
	}

    void PhysicsSystem::Init()
    {
		// 1. Create new PhysX scene
		PxSceneDesc scene_desc(PxGetPhysics().getTolerancesScale());
		scene_desc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
		scene_desc.cpuDispatcher = PxDefaultCpuDispatcherCreate(2);
		scene_desc.filterShader = CustomFilterShader;
		m_PhysicsScene = PxGetPhysics().createScene(scene_desc);

    #ifdef _DEBUG

		PxPvdSceneClient* pvd_client = m_PhysicsScene->getScenePvdClient();
		if (pvd_client)
		{
			pvd_client->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
			pvd_client->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
			pvd_client->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
		}
        
    #endif

		if (!m_Callback) 
        {
			m_Callback = std::make_unique<CollisionCallback>();
			m_PhysicsScene->setSimulationEventCallback(m_Callback.get());
		}
        
        ClearMarkedEntities();
    }

    void PhysicsSystem::Shutdown()
    {
        m_Callback.reset();
        m_Callback = nullptr;

        if (m_PhysicsScene)
        {
            m_PhysicsScene->release();
            m_PhysicsScene = nullptr;
        }
    }
    
    void PhysicsSystem::OnUpdate()
    {
        // 1. Validate Rigidbodies and Child Shapes
        for (auto it = m_RigidbodyEntitiesAddedThisFrame.begin(); it != m_RigidbodyEntitiesAddedThisFrame.end(); ++it)
        {
            Entity entity = *it;
            auto rigid_component = entity.TryGetComponent<RigidbodyComponent>();
            if (!entity || !rigid_component || !rigid_component->GetRigid() || !rigid_component->GetRigid()->GetHandle())
                continue;
            
            auto scene_manager = Application::GetProject()->GetSceneManager();
            std::function<void(Entity&)> search_self_children_for_colliders;
            search_self_children_for_colliders = [&](Entity& current_entity)
            {
                // If child and has rigidbody component, stop here and return,
                // any further child colliders won't be impacted by this
                // rigidbody.
                if (current_entity != entity && current_entity.HasComponent<RigidbodyComponent>())
                    return;

                // If has any Collider component
                if (current_entity.HasAnyComponent<
                    BoxColliderComponent, 
                    SphereColliderComponent, 
                    CapsuleColliderComponent, 
                    ConvexMeshColliderComponent, 
                    HeightFieldColliderComponent, 
                    TriangleMeshColliderComponent>())
                {

                    if (auto collider_component = current_entity.TryGetComponent<BoxColliderComponent>(); collider_component)
                    {
                        rigid_component->GetRigid()->AddShapeReference(current_entity, ShapeType_Box);
                        collider_component->GetShape()->DestroyLocalRigidDynamic();
                    }

                    if (auto collider_component = current_entity.TryGetComponent<SphereColliderComponent>(); collider_component)
                    {
                        rigid_component->GetRigid()->AddShapeReference(current_entity, ShapeType_Sphere);
                        collider_component->GetShape()->DestroyLocalRigidDynamic();
                    }

                    if (auto collider_component = current_entity.TryGetComponent<CapsuleColliderComponent>(); collider_component)
                    {
                        rigid_component->GetRigid()->AddShapeReference(current_entity, ShapeType_Capsule);
                        collider_component->GetShape()->DestroyLocalRigidDynamic();
                    }

                    if (auto collider_component = current_entity.TryGetComponent<ConvexMeshColliderComponent>(); collider_component)
                    {
                        rigid_component->GetRigid()->AddShapeReference(current_entity, ShapeType_ConvexMesh);
                        collider_component->GetShape()->DestroyLocalRigidDynamic();
                    }

                    if (auto collider_component = current_entity.TryGetComponent<HeightFieldColliderComponent>(); collider_component)
                    {
                        rigid_component->GetRigid()->AddShapeReference(current_entity, ShapeType_HeightField);
                        collider_component->GetShape()->DestroyLocalRigidDynamic();
                    }

                    if (auto collider_component = current_entity.TryGetComponent<TriangleMeshColliderComponent>(); collider_component)
                    {
                        rigid_component->GetRigid()->AddShapeReference(current_entity, ShapeType_TriangleMesh);
                        collider_component->GetShape()->DestroyLocalRigidDynamic();
                    }
                }

                for (const auto& child_guid : current_entity.GetComponent<MetaComponent>().GetChildrenGUID())
                {
                    Entity child_entity = scene_manager->GetEntity(child_guid);
                    if (!child_entity)
                        continue;
                    
                    search_self_children_for_colliders(child_entity);
                }
            };
        
            search_self_children_for_colliders(entity);
        }
        m_RigidbodyEntitiesAddedThisFrame.clear();

        // 2. Validate Shapes and Attached Rigidbody - attach lost shapes,
        //    attach new shapes, etc.
        std::vector<std::pair<Entity, ShapeType>> collider_with_no_rigid_ancestors{};
        for (auto it = m_ColliderEntitiesAddedThisFrame.begin(); it != m_ColliderEntitiesAddedThisFrame.end(); ++it)
        {
            auto& [entity, shape_type] = *it;
            if (!entity || shape_type == ShapeType_Unknown)
                continue;
            
            // Find suitable parent
            std::function<void(Entity&)> search_self_parents_for_rigidbody;
            search_self_parents_for_rigidbody = [&](Entity& current_entity)
            {
                if (current_entity.HasComponent<RigidbodyComponent>())
                {
                    // Found suitable parent
                    auto& rigidbody_component = current_entity.GetComponent<RigidbodyComponent>();
                    if (!rigidbody_component.GetRigid() || !rigidbody_component.GetRigid()->GetHandle())
                    {
                        collider_with_no_rigid_ancestors.emplace_back(entity, shape_type);
                        return;
                    }

                    rigidbody_component.GetRigid()->AddShapeReference(entity, shape_type);
                }
                else
                {
                    // Continue searching upwards
                    auto& meta_component = current_entity.GetComponent<MetaComponent>();
                    if (!meta_component.HasParent())
                    {
                        collider_with_no_rigid_ancestors.emplace_back(entity, shape_type);
                        return;
                    }

                    Entity parent_entity = Application::GetProject()->GetSceneManager()->GetEntity(meta_component.GetParentGUID());
                    if (!parent_entity)
                    {
                        collider_with_no_rigid_ancestors.emplace_back(entity, shape_type);
                        return;
                    }

                    search_self_parents_for_rigidbody(parent_entity);
                }
            };
            
            search_self_parents_for_rigidbody(entity);
        }
        m_ColliderEntitiesAddedThisFrame.clear();

        // 3. Create Local Rigidbodies for Colliders with No Suitable Parents w/ Rigidbodies
        for (auto& [entity, shape_type] : collider_with_no_rigid_ancestors)
        {
            if (!entity)
                continue;

            switch (shape_type)
            {
                case ShapeType_Box:
                {
                    if (auto component = entity.TryGetComponent<BoxColliderComponent>(); component)
                    {
                        component->GetShape()->CreateLocalRigidDynamic(entity);
                        auto local_rigid_dynamic = component->GetShape()->GetStandaloneRigidDynamic().lock();
                        local_rigid_dynamic->AddShapeReference(entity, shape_type);
                        component->GetShape()->GetHandle()->setLocalPose(PxTransform(0.0f, 0.0f, 0.0f));
                    }
                    break;
                }
                case ShapeType_Sphere:
                {
                    if (auto component = entity.TryGetComponent<SphereColliderComponent>(); component)
                    {
                        component->GetShape()->CreateLocalRigidDynamic(entity);
                        auto local_rigid_dynamic = component->GetShape()->GetStandaloneRigidDynamic().lock();
                        local_rigid_dynamic->AddShapeReference(entity, shape_type);
                        component->GetShape()->GetHandle()->setLocalPose(PxTransform(0.0f, 0.0f, 0.0f));
                    }
                    break;
                }
                case ShapeType_Capsule:
                {
                    if (auto component = entity.TryGetComponent<CapsuleColliderComponent>(); component)
                    {
                        component->GetShape()->CreateLocalRigidDynamic(entity);
                        auto local_rigid_dynamic = component->GetShape()->GetStandaloneRigidDynamic().lock();
                        local_rigid_dynamic->AddShapeReference(entity, shape_type);
                        component->GetShape()->GetHandle()->setLocalPose(PxTransform(0.0f, 0.0f, 0.0f, PxQuat(PxPiDivTwo, PxVec3(0, 0, 1)))); // Ensure capsule is YUP
                    }
                    break;
                }
                case ShapeType_ConvexMesh:
                {
                    if (auto component = entity.TryGetComponent<ConvexMeshColliderComponent>(); component)
                    {
                        component->GetShape()->CreateLocalRigidDynamic(entity);
                        auto local_rigid_dynamic = component->GetShape()->GetStandaloneRigidDynamic().lock();
                        local_rigid_dynamic->AddShapeReference(entity, shape_type);
                        component->GetShape()->GetHandle()->setLocalPose(PxTransform(0.0f, 0.0f, 0.0f));
                    }
                    break;
                }
                case ShapeType_HeightField:
                {
                    if (auto component = entity.TryGetComponent<HeightFieldColliderComponent>(); component)
                    {
                        component->GetShape()->CreateLocalRigidDynamic(entity);
                        auto local_rigid_dynamic = component->GetShape()->GetStandaloneRigidDynamic().lock();
                        local_rigid_dynamic->AddShapeReference(entity, shape_type);
                        component->GetShape()->GetHandle()->setLocalPose(PxTransform(0.0f, 0.0f, 0.0f));
                    }
                    break;
                }
                case ShapeType_TriangleMesh:
                {
                    if (auto component = entity.TryGetComponent<TriangleMeshColliderComponent>(); component)
                    {
                        component->GetShape()->CreateLocalRigidDynamic(entity);
                        auto local_rigid_dynamic = component->GetShape()->GetStandaloneRigidDynamic().lock();
                        local_rigid_dynamic->AddShapeReference(entity, shape_type);
                        component->GetShape()->GetHandle()->setLocalPose(PxTransform(0.0f, 0.0f, 0.0f));
                    }
                    break;
                }
            }
        }
    
        // 4. Update Shape's With Scale Changes
        std::unique_lock<std::mutex> lock(m_ManualUpdateScaleShapeMutex);
        for (auto& entity : m_ManualUpdatedScaleShapeEntities)
        {
            if (!entity)
                continue;
            
            glm::vec3 global_scale = entity.GetTransform().GetGlobalScale();

            if (auto component = entity.TryGetComponent<BoxColliderComponent>(); component)
            {
                glm::vec3 final_extent = glm::abs(component->GetShape()->GetHalfExtent());
                final_extent *= glm::abs(global_scale);
                component->GetShape()->UpdateBoxShape(final_extent);
            }
            
            if (auto component = entity.TryGetComponent<SphereColliderComponent>(); component)
            {
                float final_radius = glm::abs(component->GetShape()->GetRadius());
                final_radius *= glm::compMax(glm::abs(global_scale));
                component->GetShape()->UpdateSphereShape(final_radius);
            }
            
            if (auto component = entity.TryGetComponent<CapsuleColliderComponent>(); component)
            {
                float final_radius = component->GetShape()->GetRadius() * glm::max(global_scale.x, global_scale.z);
                float final_half_height = component->GetShape()->GetHalfHeight() * global_scale.y;
                component->GetShape()->UpdateCapsuleShape(final_radius, final_half_height);
            }
            
            if (auto component = entity.TryGetComponent<ConvexMeshColliderComponent>(); component)
            {
                
            }
            
            if (auto component = entity.TryGetComponent<HeightFieldColliderComponent>(); component)
            {
                
            }
            
            if (auto component = entity.TryGetComponent<TriangleMeshColliderComponent>(); component)
            {
                
            }
        }
        m_ManualUpdatedScaleShapeEntities.clear();
    }

    void PhysicsSystem::OnSimulate()
    {
        auto scene_mgr_ref = Application::GetProject()->GetSceneManager();
        if (!scene_mgr_ref || scene_mgr_ref->IsPaused() || !m_PhysicsScene)
            return;
        
        auto view = scene_mgr_ref->GetAllEntitiesWithComponent<RigidbodyComponent>();
        for (const auto& entity : view)
        {
            if (!entity)
                continue;

            auto& rigid_component = entity.GetComponent<RigidbodyComponent>();
            auto rigid_dynamic = rigid_component.GetRigid();
            if (!rigid_dynamic->IsValid())
            {
                // As we are running and this Rigid is invalid, we will init manually
                rigid_dynamic->Init(entity);
                Application::GetProject()->GetSceneManager()->GetPhysicsSystem()->RegisterRigid(entity, rigid_dynamic->GetHandle());

                // If still invalid, skip
                if (!rigid_dynamic->IsValid())
                    continue;
            }
            
            rigid_dynamic->ApplyDeferredForces();
        }

        m_PhysicsSimulating.store(true);
        m_PhysicsScene->simulate(Time::GetUnscaledFixedDeltaTime());
        m_PhysicsScene->fetchResults(true);
        m_PhysicsSimulating.store(false);
    }

    template<typename T>
    static bool UpdateColliderTransforms(T& collider_component, Entity this_entity)
    {
        GUID entity_guid = this_entity.GetGUID();

        auto physics_shape = collider_component.GetShape();
        if (!physics_shape)
            return false;
        
        PxShape* shape_handle = physics_shape->GetHandle();
        if (!shape_handle)
            return false;

        PxActor* actor = shape_handle->getActor();
        if (!actor || !actor->is<PxRigidDynamic>())
            return false;

        PxRigidDynamic* rigid_handle = reinterpret_cast<PxRigidDynamic*>(actor);
        if (!rigid_handle)
            return false;

        GUID rigid_guid = static_cast<GUID>(reinterpret_cast<uintptr_t>(rigid_handle->userData));
        Entity rigid_entity = Application::GetProject()->GetSceneManager()->GetEntity(rigid_guid);
        if (!rigid_entity)
            return false;
        
        const glm::vec3& collider_centre = physics_shape->GetCentre();

        if (entity_guid == rigid_guid)
        {
            // This has no parent rigidbody — update collider's local actor global pose
            glm::mat4 transform = this_entity.GetTransform().GetGlobalMatrix();

            glm::vec3 final_position = TransformComponent::GetPositionFromMatrix(transform);
            glm::quat final_orientation = TransformComponent::GetOrientationFromMatrix(transform);

            PxQuat entity_q(final_orientation.x, final_orientation.y, final_orientation.z, final_orientation.w);
            entity_q.normalize();
            rigid_handle->setGlobalPose(PxTransform(final_position.x, final_position.y, final_position.z, entity_q));
            
            if (physics_shape->GetType() == ShapeType_Capsule)
            {
                // Capsule is aligned along X, rotate it to align with engine Y-up
                PxQuat xup_to_yup = PxQuat(PxPiDivTwo, PxVec3(0, 0, 1));
                xup_to_yup.normalize();

                shape_handle->setLocalPose(PxTransform(
                    PxVec3(collider_centre.x, collider_centre.y, collider_centre.z),
                    xup_to_yup
                ));
            }
            else
            {
                shape_handle->setLocalPose(PxTransform(collider_centre.x, collider_centre.y, collider_centre.z));
            }
            if (auto physics_shape_ref = physics_shape->GetStandaloneRigidDynamic().lock(); physics_shape_ref)
                physics_shape_ref->SetUsingAutoCentreMass(true);
        }
        else
        {
            // Collider is on a different entity — update shape's local pose relative to rigid
            glm::mat4 shape_world = this_entity.GetTransform().GetGlobalMatrix();
            shape_world = glm::translate(shape_world, collider_centre);

            glm::mat4 rigid_world = rigid_entity.GetTransform().GetGlobalMatrix();
            glm::mat4 local_matrix = glm::inverse(rigid_world) * shape_world;

            glm::vec3 final_position = TransformComponent::GetPositionFromMatrix(local_matrix);
            glm::quat final_orientation = TransformComponent::GetOrientationFromMatrix(local_matrix);

            PxQuat entity_q(final_orientation.x, final_orientation.y, final_orientation.z, final_orientation.w);
            entity_q.normalize();

            if (physics_shape->GetType() == ShapeType_Capsule)
            {
                // Rotate capsule from X-up (PhysX) to Y-up (engine local)
                PxQuat xup_to_yup = PxQuat(PxPiDivTwo, PxVec3(0, 0, 1));
                xup_to_yup.normalize();

                // Apply XUP → YUP after computing local rotation
                PxQuat shape_rotation = entity_q * xup_to_yup;
                shape_rotation.normalize();

                shape_handle->setLocalPose(PxTransform(
                    PxVec3(final_position.x, final_position.y, final_position.z),
                    shape_rotation
                ));
            }
            else
            {
                shape_handle->setLocalPose(PxTransform(
                    final_position.x, final_position.y, final_position.z,
                    entity_q
                ));
            }

            if (auto component = rigid_entity.TryGetComponent<RigidbodyComponent>(); component)
            {
                component->GetRigid()->SetUsingAutoCentreMass(component->GetRigid()->IsUsingAutoCentreMass());
                if (!component->GetRigid()->IsKinematic())
                {
                    component->GetRigid()->GetHandle()->wakeUp();
                }
            }
        }
        return true;
    }

    template bool UpdateColliderTransforms<BoxColliderComponent>(BoxColliderComponent&, Entity);
    template bool UpdateColliderTransforms<SphereColliderComponent>(SphereColliderComponent&, Entity);
    template bool UpdateColliderTransforms<CapsuleColliderComponent>(CapsuleColliderComponent&, Entity);
    template bool UpdateColliderTransforms<ConvexMeshColliderComponent>(ConvexMeshColliderComponent&, Entity);
    template bool UpdateColliderTransforms<HeightFieldColliderComponent>(HeightFieldColliderComponent&, Entity);
    template bool UpdateColliderTransforms<TriangleMeshColliderComponent>(TriangleMeshColliderComponent&, Entity);

    void PhysicsSystem::OnTransformUpdate()
    {
        auto scene_mgr_ref = Application::GetProject()->GetSceneManager();
        if (!scene_mgr_ref || scene_mgr_ref->IsPaused() || !m_PhysicsScene)
            return;
        
        // 1. Update All Rigidbody Transforms in TransformComponent or in PxRigidDynamic::setGlobalPose

        std::unordered_set<Entity> manual_updated_rigid_transforms;
        {
            std::unique_lock<std::mutex> lock(m_ManualUpdateRigidMutex);
            manual_updated_rigid_transforms = m_ManualUpdatedRigidEntities;
        }

        auto view = scene_mgr_ref->GetAllEntitiesWithComponent<MetaComponent, RigidbodyComponent>();
        for (const auto& entity : view)
        {
            if (!entity)
                continue;

            auto& rigid_component = entity.GetComponent<RigidbodyComponent>();
            auto rigid_dynamic = rigid_component.GetRigid();
            if (!rigid_dynamic->IsValid())
            {
                // As we are running and this Rigid is invalid, we will init manually
                rigid_dynamic->Init(entity);
                Application::GetProject()->GetSceneManager()->GetPhysicsSystem()->RegisterRigid(entity, rigid_dynamic->GetHandle());

                // If still invalid, skip
                if (!rigid_dynamic->IsValid())
                    continue;
            }
            
            if (manual_updated_rigid_transforms.contains(entity))
            {
                // Entity Transform Manually Updated by TransformComponent this Frame
                auto& transform_component = entity.GetComponent<TransformComponent>();

                auto global_position = transform_component.GetGlobalPosition();
                auto global_orientation = transform_component.GetGlobalOrientation();

                PxTransform physics_transform = {};
                physics_transform.p = { global_position.x, global_position.y, global_position.z };
                physics_transform.q = { global_orientation.x, global_orientation.y, global_orientation.z, global_orientation.w };

                rigid_dynamic->GetHandle()->setGlobalPose(physics_transform);
            }
            else
            {
                // Apply Physics Simulation Change to TransformComponent - No Manual Change via TransformComponent
                auto& transform_component = entity.GetComponent<TransformComponent>();

                PxTransform physics_transform = rigid_dynamic->GetHandle()->getGlobalPose();
                transform_component.SetPosition(glm::vec3(physics_transform.p.x, physics_transform.p.y, physics_transform.p.z), true);
                transform_component.SetOrientation(glm::quat(physics_transform.q.w, physics_transform.q.x, physics_transform.q.y, physics_transform.q.z), true);
            }
        }
        manual_updated_rigid_transforms.clear();
        
        {
            std::unique_lock<std::mutex> lock(m_ManualUpdateRigidMutex);
            m_ManualUpdatedRigidEntities.clear();
        }

        // 2. Update Local Pose of all Collider Components which do not contain
        //    a Rigidbody on the same entity which received manual transform
        //    updates. If the Collider Component is not attached to a parent
        //    RigidbodyComponent and is rather connected to it's own local
        //    ColliderComponent, it will update the pxGlobalPose of the local
        //    RigidDynamic on the ColliderComponent
        std::unique_lock<std::mutex> lock(m_ManualUpdateLocalShapeMutex);
        for (auto it = m_ManualUpdatedLocalShapeEntities.begin(); it != m_ManualUpdatedLocalShapeEntities.end(); )
        {
            Entity entity = *it;
            if (!entity)
            {
                it = m_ManualUpdatedLocalShapeEntities.erase(it);
                continue;
            }

            bool collider_update_failed = false;

            if (auto component = entity.TryGetComponent<BoxColliderComponent>(); component)
            {
                if (!UpdateColliderTransforms<BoxColliderComponent>(*component, entity))
                    collider_update_failed = true;
            }

            if (auto component = entity.TryGetComponent<SphereColliderComponent>(); component)
            {
                if (!UpdateColliderTransforms<SphereColliderComponent>(*component, entity))
                    collider_update_failed = true;
            }

            if (auto component = entity.TryGetComponent<CapsuleColliderComponent>(); component)
            {
                if (!UpdateColliderTransforms<CapsuleColliderComponent>(*component, entity))
                    collider_update_failed = true;
            }

            if (auto component = entity.TryGetComponent<ConvexMeshColliderComponent>(); component)
            {
                if (!UpdateColliderTransforms<ConvexMeshColliderComponent>(*component, entity))
                    collider_update_failed = true;
            }

            if (auto component = entity.TryGetComponent<HeightFieldColliderComponent>(); component)
            {
                if (!UpdateColliderTransforms<HeightFieldColliderComponent>(*component, entity))
                    collider_update_failed = true;
            }

            if (auto component = entity.TryGetComponent<TriangleMeshColliderComponent>(); component)
            {
                if (!UpdateColliderTransforms<TriangleMeshColliderComponent>(*component, entity))
                    collider_update_failed = true;
            }
            
            if (collider_update_failed)
                ++it;
            else
                it = m_ManualUpdatedLocalShapeEntities.erase(it);
        }
    }

#pragma endregion

}