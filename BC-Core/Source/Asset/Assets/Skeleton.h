#pragma once

// Core Headers
#include "Asset/Asset.h"
#include "Core/GUID.h"

#include "Util/Hash.h"

// C++ Standard Library Headers
#include <string>
#include <vector>

// External Vendor Library Headers
#include <glm/glm.hpp>

namespace BC
{
	struct BoneLayout
	{
		/// <summary>
		/// The Name of the Bone - this will be used as a key 
		/// to the AnimationClip keyframe bone information
		/// 
		/// This is because AnimationClip's are Asset's that are 
		/// generalised across various Scenes/Prefabs where bone 
		/// entities will have varying UUID's.
		/// 
		/// So we keep the Bone Layout consistent with the name 
		/// mapping from the AnimationClip imported from ASSIMP.
		/// 
		/// When animations are processed, the AnimationClip bone
		/// references will key for the bone being transformed using 
		/// the name, and will change the transform of the 
		/// associated entity according to the BoneEntityID.
		/// </summary>
		std::string BoneName = "";

		/// <summary>
		/// The BoneID Reference
		/// </summary>
		GUID BoneID = NULL_GUID;

		/// <summary>
		/// Used for transforming vertices from Model Space to Bone Space
		/// </summary>
		glm::mat4 BoneOffsetMatrix = glm::mat4(1.0f);

		/// <summary>
		/// A Recursive Vector of Children References
		/// </summary>
		std::vector<BoneLayout> BoneChildren = {};

		BoneLayout() = default;
		BoneLayout(const BoneLayout& other) = default;
		BoneLayout(BoneLayout&& other) = default;

		BoneLayout& operator=(const BoneLayout& other) = default;
		BoneLayout& operator=(BoneLayout&& other) = default;

		size_t size() const;

		BoneLayout* find(const std::string& bone_name);

	};

	struct Bone
	{
		std::string bone_name;
		GUID bone_id;

		glm::mat4 bone_offset_matrix;

		StringHash parent_bone;
		std::vector<StringHash> child_bones;
	};

	using BoneMap = std::unordered_map<StringHash, Bone>;

    class Skeleton : public Asset
    {

    public:

		

        AssetType GetType() const override { return AssetType::Skeleton; }

		const BoneMap& GetBoneMap() const { return m_BoneMap; }
		void SetBoneMap(const BoneMap& bone_map) { m_BoneMap = bone_map; }

		void AddBone(const Bone& bone, const std::string& parent_bone) { AddBone(bone, parent_bone.empty() ? NULL_GUID : Util::HashString(parent_bone)); }
		void AddBone(const Bone& bone, StringHash parent_bone)
		{
			if (parent_bone == NULL_GUID)
			{
				BC_CORE_WARN("Skeleton::AddBone: Could Not Add Bone - Parent is NULL_GUID.");
				return;
			}
		}
		
		void UpdateRootBone(const Bone& bone)
		{
			if (m_RootBone == NULL_GUID)
			{
				m_RootBone = Util::HashString(bone.bone_name);
			}
			else
			{

			}
		}

		void RemoveBone(const std::string& bone_name) { RemoveBone(Util::HashString(bone_name)); }
		void RemoveBone(StringHash bone_hash)
		{
			if (bone_hash == NULL_GUID || !m_BoneMap.contains(bone_hash))
				return;

			for (auto& child_bone_hash : m_BoneMap[bone_hash].child_bones)
			{
				RemoveBone(child_bone_hash);
			}
			m_BoneMap.erase(bone_hash);
		}

    private:

		// Preserve Skeleton in Map to Avoid Recursion
		// Key = String Hash of Bone Name
		// Value = Bone
		BoneMap m_BoneMap;

		StringHash m_RootBone = NULL_GUID;

    };

}