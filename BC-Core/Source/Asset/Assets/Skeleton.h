#pragma once

// Core Headers
#include "Asset/Asset.h"
#include "Core/GUID.h"

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

    class Skeleton : public Asset
    {

    public:

        AssetType GetType() const override { return AssetType::Skeleton; }

        BoneLayout* GetSkeletonLayout() { return &m_SkeletonLayout; }

    private:

		BoneLayout m_SkeletonLayout = {};

    };

}