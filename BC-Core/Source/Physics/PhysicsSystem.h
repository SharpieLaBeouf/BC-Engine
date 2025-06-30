#pragma once

// Core Headers
#include "Core/GUID.h"

#include "Project/Scene/Entity.h"

// C++ Standard Library Headers
#include <atomic>
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

        PxScene* GetScene() { return m_PhysicsScene; }
        bool IsPhysicsSimulating() const { return m_PhysicsSimulating.load(); }

        /// @brief Update Shape Attachments to Rigids
        void OnUpdate();

        /// @brief Perform the Physics Simulation
        void OnSimulate();

        /// @brief Update All TransformComponents Impacted by Physics Simulation, and Vice Versa, Update RigidDynamic's for Manual Changes to TransformComponent
        void OnTransformUpdate();

        /// @brief Mark an Entity as having their transform manually updated in
        /// this frame meaning it will not receive changes from physics simulation. 
        void MarkEntityRigidbodyTransformDirty(const Entity& entity)
        {
            std::unique_lock<std::mutex> lock(m_ManualUpdateRigidMutex);
            m_ManualUpdatedRigidEntities.insert(entity); 
        }

        /// @brief Mark an Entity with a Collider type component without a
        /// RigidbodyCompoennt as having their transform manually updated in
        /// this frame. This means the local pose of the shape may need to be
        /// updated relative to the parent Rigidbodies global transform matrix.
        void MarkEntityLocalShapeTransformDirty(const Entity& entity)
        {
            std::unique_lock<std::mutex> lock(m_ManualUpdateLocalShapeMutex);
            m_ManualUpdatedLocalShapeEntities.insert(entity);
        }

        /// @brief Mark an entity scale changed which means that the size of the
        /// shape will be influenced by the changed scale
        void MarkEntityShapeScaleChanged(const Entity& entity)
        {
            std::unique_lock<std::mutex> lock(m_ManualUpdateScaleShapeMutex);
            m_ManualUpdatedScaleShapeEntities.insert(entity);
        }

        /// @brief Add a rigidbody to the physics system
        void RegisterRigid(const Entity& entity, PxActor* handle)
        {
            m_RigidbodyEntitiesAddedThisFrame.insert(entity);
            handle->userData = reinterpret_cast<void*>(static_cast<uintptr_t>(entity.GetGUID()));

            if (m_PhysicsScene)
            {
                m_PhysicsScene->addActor(*handle);
            }
        }

        /// @brief Add a shape to the physics system
        void RegisterShape(const Entity& entity, PxShape* handle, ShapeType type)
        {
            m_ColliderEntitiesAddedThisFrame.emplace_back(entity, type);
            handle->userData = reinterpret_cast<void*>(static_cast<uintptr_t>(entity.GetGUID()));
        }

        void ClearMarkedEntities()
        {
            m_RigidbodyEntitiesAddedThisFrame.clear();
            m_ColliderEntitiesAddedThisFrame.clear();
        }

    private:

        /// @brief This is basically marking these entities as dirty to update
        /// shape relationships when adding in. For example, an ancestor entity
        /// may have a rigidbody. Any colliders on child entities are connected
        /// to this rigidbody implicitly. When a new rigidbody is added between
        /// these two, it will update the shape relationship, detach from
        /// previous ancestor, and attach to new rigidbody added.
        std::unordered_set<Entity> m_RigidbodyEntitiesAddedThisFrame = {};

        /// @brief This is basically marking these entities as dirty to update
        /// shape relationships when adding in. For example, an ancestor entity
        /// may have a rigidbody. Any colliders on child entities are connected
        /// to this rigidbody implicitly. When a new rigidbody is added between
        /// these two, it will update the shape relationship, detach from
        /// previous ancestor, and attach to new rigidbody added.
        std::vector<std::pair<Entity, ShapeType>> m_ColliderEntitiesAddedThisFrame = {};

        /// @brief This is a set of entities that have rigidbodies by their
        /// transform has been manually changed through TransformComponent
        std::mutex m_ManualUpdateRigidMutex;
        std::unordered_set<Entity> m_ManualUpdatedRigidEntities = {};
        
        /// @brief This is a set of Entities that do not have a Rigidbody
        /// component on the same entity which received manual transform updates
        /// requiring local pose updates relative to the parent Rigidbody / or
        /// the local RigidDynamic in the shape 
        std::mutex m_ManualUpdateLocalShapeMutex;
        std::unordered_set<Entity> m_ManualUpdatedLocalShapeEntities = {};
        
        /// @brief This is a set of Entities that require all shapes to be
        /// resized based on the new global scale of the entity.
        std::mutex m_ManualUpdateScaleShapeMutex;
        std::unordered_set<Entity> m_ManualUpdatedScaleShapeEntities = {};

        PxScene* m_PhysicsScene = nullptr;
        std::atomic<bool> m_PhysicsSimulating = false;

        std::unique_ptr<CollisionCallback> m_Callback = nullptr;

    };

}