#pragma once

#define PX_RELEASE(x) if(x)	{ x->release(); x = NULL;	}

// Core Headers

// C++ Standard Library Headers

// External Vendor Library Headers
#include <physx/PxPhysicsAPI.h>

namespace BC
{
	class Physics 
    {

	public:

		static void Init();
		static void Shutdown();

	private:

		// Delete default constructor
		Physics() = delete;

		// Delete copy assignment and move assignment constructors
		Physics(const Physics&) = delete;
		Physics(Physics&&) = delete;

		// Delete copy assignment and move assignment operators
		Physics& operator=(const Physics&) = delete;
		Physics& operator=(Physics&&) = delete;

		static physx::PxPvd* 					mPvd;
		static physx::PxPhysics* 				mPhysics ;
		static physx::PxFoundation* 			mFoundation;
		static physx::PxDefaultAllocator 		mAllocator;
		static physx::PxDefaultErrorCallback 	mErrorCallback;
		static physx::PxDefaultCpuDispatcher* 	mDispatcher;

	};
}