#pragma once

// Core Headers

// C++ Standard Library Headers
#include <cstdint>

// External Vendor Library Headers

namespace BC
{
    namespace Util
    {

    }

	enum class AssetType : uint8_t 
    {

		Unknown = 0,

		Scene,
		Prefab,

		Texture2D,
		TextureCubeMap,
		RenderTarget,

		Mesh,
		ModelImport,

		Skeleton,
		AnimationClip,
		AnimationStateMachine,

		Audio,

		Material_Standard,
		Material_Skybox,

		Compute_Shader,
		Shader,
		
		Humanoid,
		HumanoidMask

	};

    using AssetHandle = uint64_t;
    
    class Asset
    {

	public:

		AssetHandle Handle;

		virtual bool operator==(const Asset& other) const { return Handle == other.Handle; }
		virtual AssetType GetType() const = 0;

    };

	struct AssetMetaData
	{

	};

}