#include "BC_PCH.h"
#include "Physics.h"

namespace BC
{
	// Define static member variables
	physx::PxPvd* 					Physics::mPvd 			= nullptr;
	physx::PxPhysics* 				Physics::mPhysics 		= nullptr;
	physx::PxFoundation* 			Physics::mFoundation 	= nullptr;
	physx::PxDefaultAllocator 		Physics::mAllocator		= {};
	physx::PxDefaultErrorCallback 	Physics::mErrorCallback	= {};
	physx::PxDefaultCpuDispatcher* 	Physics::mDispatcher 	= nullptr;

	void Physics::Init() 
	{
		// 1. Init Physics Founation
		mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, mAllocator, mErrorCallback);
		if (!mFoundation)
        {
			BC_CORE_FATAL("PxCreateFoundation Failed!");
			return;
		}
		else
        {
			BC_CORE_INFO("PxCreateFoundation Successful!");

			// 2. Init PhysX Visual Debugger (PVD) if in debug
#ifdef _DEBUG
			mPvd = physx::PxCreatePvd(*mFoundation);
			physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
			mPvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);
#endif

			// 3. Init top level physics object
			mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, physx::PxTolerancesScale(), true, mPvd);

			if (!mPhysics)
            {
				BC_CORE_FATAL("PxCreatePhysics Failed!");

				if (mPvd)
				{
					physx::PxPvdTransport* transport = mPvd->getTransport();
					mPvd->release();
					mPvd = nullptr;
					PX_RELEASE(transport);
				}

				PX_RELEASE(mFoundation);
				mFoundation = nullptr;

				return; // Initialization failed
			}
			else 
            {
				BC_CORE_INFO("PxCreatePhysics Successful!");

				// 4. Sets up default CPU dispatcher for the PhysX simulation
				mDispatcher = physx::PxDefaultCpuDispatcherCreate(2);

				return; // Initialization succeeded
			}
		}
	}

    void Physics::Shutdown()
    {
        PX_RELEASE(mDispatcher);
        PX_RELEASE(mPhysics);
        PX_RELEASE(mPvd);
        PX_RELEASE(mFoundation);
    }

}