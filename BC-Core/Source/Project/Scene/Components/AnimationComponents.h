#pragma once

// Core Headers
#include "Component Base.h"

#include "Asset/Assets/AnimationStateMachine/AnimationStateMachine.h"

// C++ Standard Library Headers

// External Vendor Library Headers

namespace YAML 
{
	class Emitter;
	class Node;
}

namespace BC
{
	struct BoneLayout; // Forward Declare
	class AnimationClip;
	class Skeleton;

    struct SimpleAnimationComponent : public ComponentBase
    {
    
    public:

        SimpleAnimationComponent() = default;
        ~SimpleAnimationComponent() = default;

        SimpleAnimationComponent(const SimpleAnimationComponent& other);
        SimpleAnimationComponent(SimpleAnimationComponent&& other) noexcept;
        SimpleAnimationComponent& operator=(const SimpleAnimationComponent& other);
        SimpleAnimationComponent& operator=(SimpleAnimationComponent&& other) noexcept;

        ComponentType GetType() const override { return ComponentType::SimpleAnimationComponent; }

        void SceneSerialise(YAML::Emitter& out) const override;
        bool SceneDeserialise(const YAML::Node& data) override;
        
        // --- SimpleAnimationComponent Functions ---

        void Play(int clip_index, bool should_loop = true);
        void Play(const char* clip_name, bool should_loop = true);

        void Pause() { m_IsPlaying = false; }
        void Resume() { m_IsPlaying = true; }
        void Stop() { m_IsPlaying = false; m_CurrentTime = 0.0f; }
        void SetPlaybackSpeed(float speed) { m_PlaybackSpeed = speed; }

        /// @brief Use This to Update the Bone Entity Transforms Whilst Iterating Bone Hierarchy
        void Update();

        /// @brief Use This to Provide a Mapping of Entity's and their New Transforms Based on Animation Updates
        std::unordered_map<GUID, glm::mat4> UpdateDeferred();

        /// @brief Step the animation timer
        void StepAnimationTimer(const std::shared_ptr<AnimationClip>& animation_clip);

    private:

        void UpdateBoneHierarchy(const BoneLayout& current_bone, std::shared_ptr<Skeleton> skeleton_asset, const std::unordered_map<GUID, GUID>& bone_mapping, const AnimationClip& animation_clip, float time);
        void UpdateBoneHierarchyDeferred(std::unordered_map<GUID, glm::mat4>& transform_update_map, const BoneLayout& current_bone, std::shared_ptr<Skeleton> skeleton_asset, const std::unordered_map<GUID, GUID>& bone_mapping, const AnimationClip& animation_clip, float time);

    private:
        
        /// @brief A Vector of References to Animation Assets
        std::vector<AssetHandle> m_AnimationClipHandles;

        /// @brief Index of the Vector to Which Animation is Currently Playing
        int m_CurrentClipIndex = -1;

        /// @brief The Current Time of the Animation Being Played
        float m_CurrentTime = 0.0f;

        /// @brief The Speed of the Animation Playback
        float m_PlaybackSpeed = 1.0f;

        /// @brief Flag - should the animation be playing
        bool m_IsPlaying = false;

        /// @brief Flag - should the animation be looping
        bool m_IsLooping = true;

        /// @brief Determines how animation culling should behave
        enum class AnimationCullingMode : uint8_t
        {
            AlwaysAnimate = 0,					// Animations are processed
            NoAnimateOffScreenContinueTimer,	// Off-screen animations stop updating pose, but continue timer
            NoAnimateOffScreenStopTimer			// Off-screen animations stop pose and stop timer
        };

        /// @brief The culling behavior mode for this animation player
        AnimationCullingMode m_CullingMode = AnimationCullingMode::AlwaysAnimate;

    };

    struct AnimatorComponent : public ComponentBase
    {

		enum class AnimationCullingMode : uint8_t
		{
			AlwaysAnimate = 0,					// Animations are procesed 
			NoAnimateOffScreenContinueTimer,	// No Animations are processed - animation timer is continued
			NoAnimateOffScreenStopTimer			// No Animations are processed - animation timer is stopped
		};
    
    public:

        AnimatorComponent() = default;
        ~AnimatorComponent() = default;

        AnimatorComponent(const AnimatorComponent& other);
        AnimatorComponent(AnimatorComponent&& other) noexcept;
        AnimatorComponent& operator=(const AnimatorComponent& other);
        AnimatorComponent& operator=(AnimatorComponent&& other) noexcept;

        ComponentType GetType() const override { return ComponentType::AnimatorComponent; }

        void SceneSerialise(YAML::Emitter& out) const override;
        bool SceneDeserialise(const YAML::Node& data) override;

        bool Init() override;
        bool Shutdown() override;
        
        /// @brief Get the animation state machine asset handle
        AssetHandle GetStateMachineHandle() const { return m_StateMachineHandle; }

        /// @brief Set the animation state machine asset handle
        void SetStateMachineHandle(AssetHandle handle) { m_StateMachineHandle = handle; }

        /// @brief Get the animation culling mode
        AnimationCullingMode GetCullingMode() const { return m_CullingMode; }

        /// @brief Set the animation culling mode
        void SetCullingMode(AnimationCullingMode mode) { m_CullingMode = mode; }

        /// @brief Get the state machine instance
        Animation::StateMachine* GetStateMachineInstance() const { return m_StateMachineInstance.get(); }

    private:

        AssetHandle m_StateMachineHandle = NULL_GUID;

        AnimationCullingMode m_CullingMode = AnimationCullingMode::AlwaysAnimate;

        std::unique_ptr<Animation::StateMachine> m_StateMachineInstance = nullptr;
        
    };
    
}