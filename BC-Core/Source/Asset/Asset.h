#pragma once

// Core Headers

// C++ Standard Library Headers
#include <cstdint>
#include <filesystem>

// External Vendor Library Headers

namespace BC
{
    namespace Util
    {

    }

	enum class AssetType : uint8_t 
    {

		Unknown = 0,

		Prefab,			// Actual prefabs or model import files, e.g., fbx, obj, etc.

		Texture2D,
		TextureCubeMap,
		RenderTarget,

		Mesh,			// Mesh -> has sub meshes which make up entire mesh

		Skeleton,
		Humanoid,
		HumanoidMask,
		AnimationClip,
		AnimationStateMachine,

		Audio,

		Material_Standard,
		Material_Skybox,

		Compute_Shader,
		Shader,

		PhysicsMaterial

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
		/// @brief Name of the Asset
		std::string name;

		/// @brief Relative Path of Asset to Project/Assets Folder
		std::filesystem::path asset_path;

		/// @brief The Type of the Asset
		AssetType type;

		/// @brief The Handle of the Asset
		AssetHandle handle = NULL_GUID;

		/// @brief The Parent of the Asset -> some assets are created by others,
		/// e.g., models create materials, when trying to get them, you need to
		/// load the parent first
		AssetHandle parent_handle;

		/// @brief If this is a parent asset which has child references that may
		/// be derived from this asset
		bool has_children_assets;

		/// @brief If this asset is transient and is created during runtime,
		/// opposed to during editing
		bool runtime_asset;

		/// @brief Is this an inbuilt asset such as default engine assets, e.g., Default Shader, Default Material, Default Cube, etc.
		bool inbuilt_asset;

		operator bool() const { return handle == NULL_GUID || handle == PLACEHOLDER_0_GUID; } // Indicates Null MetaData
	};

}