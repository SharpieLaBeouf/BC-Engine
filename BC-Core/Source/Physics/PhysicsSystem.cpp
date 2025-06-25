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
    }

    void PhysicsSystem::Shutdown()
    {
        m_Callback.reset();
        m_Callback = nullptr;

		m_PhysicsScene->release();
		m_PhysicsScene = nullptr;
    }

    void PhysicsSystem::OnUpdate()
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
                rigid_dynamic->Init();

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

    void PhysicsSystem::OnTransformUpdate()
    {
        auto scene_mgr_ref = Application::GetProject()->GetSceneManager();
        if (!scene_mgr_ref || scene_mgr_ref->IsPaused() || !m_PhysicsScene)
            return;
        
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
                rigid_dynamic->Init();

                // If still invalid, skip
                if (!rigid_dynamic->IsValid())
                    continue;
            }

            if (m_ManualUpdatedRigidEntities.contains(entity))
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
        
        m_ManualUpdatedRigidEntities.clear();
    }

#pragma endregion

}