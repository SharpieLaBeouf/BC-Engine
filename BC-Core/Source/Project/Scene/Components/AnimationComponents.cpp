#include "BC_PCH.h"
#include "AnimationComponents.h"

#include "Asset/AssetManagerAPI.h"
#include "Asset/Assets/AnimationClip.h"
#include "Asset/Assets/Skeleton.h"
#include "Asset/Assets/AnimationStateMachine/AnimationStateMachine.h"

#include <yaml-cpp/yaml.h>

namespace BC
{
    
#pragma region SimpleAnimationComponent
    
    SimpleAnimationComponent::SimpleAnimationComponent(const SimpleAnimationComponent& other)
    {
        m_AnimationClipHandles = other.m_AnimationClipHandles;
        m_CurrentClipIndex = other.m_CurrentClipIndex;
        m_CurrentTime = other.m_CurrentTime;
        m_PlaybackSpeed = other.m_PlaybackSpeed;
        m_IsPlaying = other.m_IsPlaying;
        m_IsLooping = other.m_IsLooping;
        m_CullingMode = other.m_CullingMode;
    }

    SimpleAnimationComponent::SimpleAnimationComponent(SimpleAnimationComponent&& other) noexcept
    {
		m_Entity = std::move(other.m_Entity); other.m_Entity = nullptr;

        m_AnimationClipHandles = std::move(other.m_AnimationClipHandles);
        m_CurrentClipIndex = other.m_CurrentClipIndex;
        m_CurrentTime = other.m_CurrentTime;
        m_PlaybackSpeed = other.m_PlaybackSpeed;
        m_IsPlaying = other.m_IsPlaying;
        m_IsLooping = other.m_IsLooping;
        m_CullingMode = other.m_CullingMode;

        other.m_CurrentClipIndex = -1;
        other.m_CurrentTime = 0.0f;
        other.m_PlaybackSpeed = 1.0f;
        other.m_IsPlaying = false;
        other.m_IsLooping = true;
        other.m_CullingMode = AnimationCullingMode::AlwaysAnimate;
    }

    SimpleAnimationComponent& SimpleAnimationComponent::operator=(const SimpleAnimationComponent& other)
    {
        if (this == &other)
            return *this;

        m_AnimationClipHandles = other.m_AnimationClipHandles;
        m_CurrentClipIndex = other.m_CurrentClipIndex;
        m_CurrentTime = other.m_CurrentTime;
        m_PlaybackSpeed = other.m_PlaybackSpeed;
        m_IsPlaying = other.m_IsPlaying;
        m_IsLooping = other.m_IsLooping;
        m_CullingMode = other.m_CullingMode;

        return *this;
    }

    SimpleAnimationComponent& SimpleAnimationComponent::operator=(SimpleAnimationComponent&& other) noexcept
    {
        if (this == &other)
            return *this;

		m_Entity = std::move(other.m_Entity); other.m_Entity = nullptr;

        m_AnimationClipHandles = std::move(other.m_AnimationClipHandles);
        m_CurrentClipIndex = other.m_CurrentClipIndex;
        m_CurrentTime = other.m_CurrentTime;
        m_PlaybackSpeed = other.m_PlaybackSpeed;
        m_IsPlaying = other.m_IsPlaying;
        m_IsLooping = other.m_IsLooping;
        m_CullingMode = other.m_CullingMode;

        other.m_CurrentClipIndex = -1;
        other.m_CurrentTime = 0.0f;
        other.m_PlaybackSpeed = 1.0f;
        other.m_IsPlaying = false;
        other.m_IsLooping = true;
        other.m_CullingMode = AnimationCullingMode::AlwaysAnimate;

        return *this;
    }

    void SimpleAnimationComponent::SceneSerialise(YAML::Emitter& out) const
    {
        out << YAML::Key << Util::ComponentTypeToString(GetType()) << YAML::Value;
		out << YAML::BeginMap;

		out << YAML::Key << "CurrentClipIndex" << YAML::Value << m_CurrentClipIndex;

		out << YAML::Key << "CurrentTime" << YAML::Value << m_CurrentTime;
		out << YAML::Key << "PlaybackSpeed" << YAML::Value << m_PlaybackSpeed;

		out << YAML::Key << "IsPlaying" << YAML::Value << m_IsPlaying;
		out << YAML::Key << "IsLooping" << YAML::Value << m_IsLooping;

		switch (m_CullingMode)
		{
			case AnimationCullingMode::AlwaysAnimate:
			{
				out << YAML::Key << "CullingMode" << YAML::Value << "AlwaysAnimate";
				break;
			}
			case AnimationCullingMode::NoAnimateOffScreenContinueTimer:
			{
				out << YAML::Key << "CullingMode" << YAML::Value << "NoAnimateOffScreenContinueTimer";
				break;
			}
			default:
			case AnimationCullingMode::NoAnimateOffScreenStopTimer:
			{
				out << YAML::Key << "CullingMode" << YAML::Value << "NoAnimateOffScreenStopTimer";
				break;
			}
		}

        out << YAML::Key << "AnimationClipHandles" << YAML::Value;
        {
            out << YAML::BeginSeq;
            for (const auto& handle : m_AnimationClipHandles) 
            {
                out << (uint64_t)handle;
            }
            out << YAML::EndSeq;
        }

		out << YAML::EndMap;
    }

    bool SimpleAnimationComponent::SceneDeserialise(const YAML::Node& data)
    {
		if (data["CurrentClipIndex"]) 
		{
			m_CurrentClipIndex = data["CurrentClipIndex"].as<int>();
		}

		if (data["CurrentTime"]) 
		{
			m_CurrentTime = data["CurrentTime"].as<float>();
		}

		if (data["PlaybackSpeed"]) 
		{
			m_PlaybackSpeed = data["PlaybackSpeed"].as<float>();
		}

		if (data["IsPlaying"]) 
		{
			m_IsPlaying = data["IsPlaying"].as<bool>();
		}

		if (data["IsLooping"]) 
		{
			m_IsLooping = data["IsLooping"].as<bool>();
		}

		if (data["CullingMode"]) 
		{
			std::string culling_mode = data["CullingMode"].as<std::string>();

			if (culling_mode == "AlwaysAnimate")
				m_CullingMode = AnimationCullingMode::AlwaysAnimate;
			else if (culling_mode == "NoAnimateOffScreenContinueTimer")
				m_CullingMode = AnimationCullingMode::NoAnimateOffScreenContinueTimer;
			else if (culling_mode == "NoAnimateOffScreenStopTimer")
				m_CullingMode = AnimationCullingMode::NoAnimateOffScreenStopTimer;
		}

		if (data["AnimationClipHandles"]) 
		{
			YAML::Node handles = data["AnimationClipHandles"];
			m_AnimationClipHandles.clear();
			for (const auto& handle : handles) 
			{
				m_AnimationClipHandles.emplace_back(handle.as<uint64_t>());
			}
		}
		else 
		{
			return false;
		}

		return true;
    }

    void SimpleAnimationComponent::Play(int clip_index, bool should_loop)
    {
		if (clip_index < 0 || clip_index >= m_AnimationClipHandles.size())
		{
			BC_CORE_WARN("BasicAnimationComponent::Play - Clip Index Invalid.");
			return;
		}

		m_CurrentClipIndex = clip_index;
		m_CurrentTime = 0.0f;
		m_IsPlaying = true;
		m_IsLooping = should_loop;
    }

    void SimpleAnimationComponent::Play(const char *clip_name, bool should_loop)
    {
		uint32_t found_index = NULL_GUID;
		for (auto it = m_AnimationClipHandles.begin(); it != m_AnimationClipHandles.end(); )
		{
			if (*it == NULL_GUID)
			{
				++it;
				continue;
			}

            // TODO: Implement
			// const auto& clip_meta_data = Project::GetActiveProject()->GetEditorAssetManager()->GetMetadata(*it);
			// if(clip_meta_data.AssetName == clip_name)
			// {
			// 	found_index = std::distance(AnimationClipHandles.begin(), it);
			// 	break;
			// }
			++it;
		}

		if(found_index == NULL_GUID)
		{
			BC_CORE_WARN("BasicAnimationComponent::Play - Could Not Find \"{}\" in Animation Clips.", clip_name);
			return;
		}

		m_CurrentClipIndex = found_index;
		m_CurrentTime = 0.0f;
		m_IsPlaying = true;
		m_IsLooping = should_loop;
    }

    void SimpleAnimationComponent::Update()
    {
		// Check if Valid State to Animate
		if (!m_IsPlaying || m_CurrentClipIndex == -1) 
		{
			return;
		}

		// Retrieve Animation Clip Asset
		std::shared_ptr<AnimationClip> animation_clip_asset = nullptr;

		if (!AssetManager::IsAssetLoaded(m_AnimationClipHandles[m_CurrentClipIndex]))
			return;

		animation_clip_asset = AssetManager::GetAsset<AnimationClip>(m_AnimationClipHandles[m_CurrentClipIndex]);

		StepAnimationTimer(animation_clip_asset);

		// Get Skinned Mesh Component
		auto& skinned_mesh_component = GetComponent<SkinnedMeshRendererComponent>();

		// Get Skeleton Asset
		auto asset_skeleton = AssetManager::GetAsset<Skeleton>(skinned_mesh_component.GetSkeleton());
        if (!asset_skeleton)
            return;

		// Update Bone Hierarchy of Skeleton
		UpdateBoneHierarchy(*asset_skeleton->GetSkeletonLayout(), asset_skeleton, *skinned_mesh_component.GetBoneMapping(), *animation_clip_asset.get(), m_CurrentTime);
    }

    std::unordered_map<GUID, glm::mat4> SimpleAnimationComponent::UpdateDeferred()
    {
		// Check if Valid State to Animate
		if (!m_IsPlaying || m_CurrentClipIndex == -1)
		{
			return {};
		}

		// Retrieve Animation Clip Asset
		std::shared_ptr<AnimationClip> animation_clip_asset = nullptr;

		if (!AssetManager::IsAssetLoaded(m_AnimationClipHandles[m_CurrentClipIndex]))
			return {};

		animation_clip_asset = AssetManager::GetAsset<AnimationClip>(m_AnimationClipHandles[m_CurrentClipIndex]);

		// Increment Current Animation Time
		m_CurrentTime += Time::GetDeltaTime() * animation_clip_asset->GetTicksPerSecond() * m_PlaybackSpeed;

		// Loop of Stop Animation
		if (m_CurrentTime > animation_clip_asset->GetDuration())
		{
			if (m_IsLooping)
			{
				m_CurrentTime = 0.0f;
			}
			else
			{
				Stop();
				return {};
			}
		}

		// Get Skinned Mesh Component
		auto& skinned_mesh_component = GetComponent<SkinnedMeshRendererComponent>();

		// Get Skeleton Asset
		auto asset_skeleton = AssetManager::GetAsset<Skeleton>(skinned_mesh_component.GetSkeleton());
        if (!asset_skeleton)
            return {};

		std::unordered_map<GUID, glm::mat4> transform_update_map;
		transform_update_map.reserve(skinned_mesh_component.GetBoneMapping()->size());
		// Update Bone Hierarchy of Skeleton
		UpdateBoneHierarchyDeferred(transform_update_map, *asset_skeleton->GetSkeletonLayout(), asset_skeleton, *skinned_mesh_component.GetBoneMapping(), *animation_clip_asset.get(), m_CurrentTime);
		return transform_update_map;
    }

    void SimpleAnimationComponent::StepAnimationTimer(const std::shared_ptr<AnimationClip> &animation_clip)
    {
		// Increment Current Animation Time
		m_CurrentTime += Time::GetDeltaTime() * animation_clip->GetTicksPerSecond() * m_PlaybackSpeed;

		// Loop of Stop Animation
		if (m_CurrentTime > animation_clip->GetDuration())
		{
			if (m_IsLooping)
			{
				m_CurrentTime = 0.0f;
			}
			else
			{
				Stop();
				return;
			}
		}
    }

    void SimpleAnimationComponent::UpdateBoneHierarchy(const BoneLayout &current_bone, std::shared_ptr<Skeleton> skeleton_asset, const std::unordered_map<GUID, GUID> &bone_mapping, const AnimationClip &animation_clip, float time)
    {
        // TODO: Implement
    }

    void SimpleAnimationComponent::UpdateBoneHierarchyDeferred(std::unordered_map<GUID, glm::mat4> &transform_update_map, const BoneLayout &current_bone, std::shared_ptr<Skeleton> skeleton_asset, const std::unordered_map<GUID, GUID> &bone_mapping, const AnimationClip &animation_clip, float time)
    {
        // TODO: Implement
    }

#pragma endregion

#pragma region AnimatorComponent
    
    AnimatorComponent::AnimatorComponent(const AnimatorComponent& other)
    {
        m_StateMachineHandle = other.m_StateMachineHandle;
        m_CullingMode = other.m_CullingMode;

        if (other.m_StateMachineInstance)
            m_StateMachineInstance = std::make_unique<Animation::StateMachine>(*other.m_StateMachineInstance);
        else
            m_StateMachineInstance = nullptr;
    }

    AnimatorComponent::AnimatorComponent(AnimatorComponent&& other) noexcept
    {
		m_Entity = std::move(other.m_Entity); other.m_Entity = nullptr;

        m_StateMachineHandle = other.m_StateMachineHandle;                  other.m_StateMachineHandle = NULL_GUID;
        m_CullingMode = other.m_CullingMode;                                other.m_CullingMode = AnimationCullingMode::AlwaysAnimate;
        m_StateMachineInstance = std::move(other.m_StateMachineInstance);   other.m_StateMachineInstance = nullptr;
    }

    AnimatorComponent& AnimatorComponent::operator=(const AnimatorComponent& other)
    {
        if (this == &other)
            return *this;

        m_StateMachineHandle = other.m_StateMachineHandle;
        m_CullingMode = other.m_CullingMode;

        if (other.m_StateMachineInstance)
            m_StateMachineInstance = std::make_unique<Animation::StateMachine>(*other.m_StateMachineInstance);
        else
            m_StateMachineInstance = nullptr;

        return *this;
    }

    AnimatorComponent& AnimatorComponent::operator=(AnimatorComponent&& other) noexcept
    {
        if (this == &other)
            return *this;

		m_Entity = std::move(other.m_Entity); other.m_Entity = nullptr;

        m_StateMachineHandle = other.m_StateMachineHandle;                  other.m_StateMachineHandle = NULL_GUID;
        m_CullingMode = other.m_CullingMode;                                other.m_CullingMode = AnimationCullingMode::AlwaysAnimate;
        m_StateMachineInstance = std::move(other.m_StateMachineInstance);   other.m_StateMachineInstance = nullptr;

        return *this;
    }
    
    bool AnimatorComponent::Init()
    {
		m_StateMachineInstance.reset();
		m_StateMachineInstance = nullptr;

		if (m_StateMachineHandle != NULL_GUID && AssetManager::IsAssetHandleValid(m_StateMachineHandle))
		{
			auto machine_asset = AssetManager::GetAsset<Animation::StateMachine>(m_StateMachineHandle);
			if(machine_asset)
			{
				m_StateMachineInstance = std::make_unique<Animation::StateMachine>(*machine_asset.get()); // Copy asset into component instance
			}
		}
		return true;
    }
    
    bool AnimatorComponent::Shutdown()
    {
		m_StateMachineInstance.reset();
		m_StateMachineInstance = nullptr;
		return true;
    }

    void AnimatorComponent::SceneSerialise(YAML::Emitter& out) const
    {
        out << YAML::Key << Util::ComponentTypeToString(GetType()) << YAML::Value;
		out << YAML::BeginMap;
		{
			out << YAML::Key << "StateMachineHandle" << YAML::Value << m_StateMachineHandle;

			switch (m_CullingMode)
			{
				case AnimationCullingMode::AlwaysAnimate:
				{
					out << YAML::Key << "CullingMode" << YAML::Value << "AlwaysAnimate";
					break;
				}
				case AnimationCullingMode::NoAnimateOffScreenContinueTimer:
				{
					out << YAML::Key << "CullingMode" << YAML::Value << "NoAnimateOffScreenContinueTimer";
					break;
				}
				default:
				case AnimationCullingMode::NoAnimateOffScreenStopTimer:
				{
					out << YAML::Key << "CullingMode" << YAML::Value << "NoAnimateOffScreenStopTimer";
					break;
				}
			}

		}
		out << YAML::EndMap;
    }

    bool AnimatorComponent::SceneDeserialise(const YAML::Node& data)
    {
		YAML::Node component = data;

		if (component["StateMachineHandle"]) 
		{
			m_StateMachineHandle = component["StateMachineHandle"].as<uint32_t>();
		}

		if (component["CullingMode"]) 
		{
			std::string culling_mode = component["CullingMode"].as<std::string>();

			if (culling_mode == "AlwaysAnimate")
				m_CullingMode = AnimationCullingMode::AlwaysAnimate;
			else if (culling_mode == "NoAnimateOffScreenContinueTimer")
				m_CullingMode = AnimationCullingMode::NoAnimateOffScreenContinueTimer;
			else if (culling_mode == "NoAnimateOffScreenStopTimer")
				m_CullingMode = AnimationCullingMode::NoAnimateOffScreenStopTimer;
		}

		return true;
    }

#pragma endregion

}