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
		PxPvdSceneClient* pvd_client = m_PhysxScene->getScenePvdClient();
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

        // TODO: Implement Initialise Physics Components
    }


    void PhysicsSystem::Shutdown()
    {

        // TODO: Implement Shutdown Physics Components



        m_Callback.reset();
        m_Callback = nullptr;

		m_PhysicsScene->release();
		m_PhysicsScene = nullptr;
    }

#pragma endregion

}