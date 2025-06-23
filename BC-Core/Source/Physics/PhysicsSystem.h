#pragma once

// Core Headers

// C++ Standard Library Headers
#include <memory>
#include <unordered_set>

// External Vendor Library Headers
#include <physx/PxPhysicsAPI.h>

using namespace physx;

namespace BC
{

    class CollisionCallback : public PxSimulationEventCallback
    {
		struct CollisionPairHash 
        {
			template <class T1, class T2>
			std::size_t operator()(const std::pair<T1, T2>& pair) const 
            {
				return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
			}
		};

	public:

		CollisionCallback() = default;

		void onContact(const PxContactPairHeader& pair_header, const PxContactPair* pairs, PxU32 pairs_count) override;
		void onTrigger(PxTriggerPair*, PxU32) override;
		void onConstraintBreak(PxConstraintInfo*, PxU32) override;
		void onAdvance(const PxRigidBody* const* body_buffer, const PxTransform* pose_buffer, const PxU32 count) override;
		void onWake(PxActor**, PxU32) override;
		void onSleep(PxActor**, PxU32) override;

		inline static std::unordered_set<std::pair<uint32_t, uint32_t>, CollisionPairHash> s_ActiveTriggers;

    };

    class PhysicsSystem
    {

    public:

        PhysicsSystem() = default;
        ~PhysicsSystem() { Shutdown(); }

        void Init();
        void Shutdown();

    private:

        PxScene* m_PhysicsScene = nullptr;

        std::unique_ptr<CollisionCallback> m_Callback = nullptr;

    };

}