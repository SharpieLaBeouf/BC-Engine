#pragma once

// Core Headers

// C++ Standard Library Headers
#include <cstdint>
#include <filesystem>

// External Vendor Library Headers

namespace BC
{
	enum class AssetType : uint8_t 
    {

		Unknown = 0,

		PrefabEntity,	// Actual prefabs or model import files, e.g., fbx, obj, etc.
		PrefabModel,	// Actual prefabs or model import files, e.g., fbx, obj, etc.

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

		Material,
		Skybox,

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

	struct AssetImportConfig 
	{
		using AssetImportType = uint8_t;
		enum : AssetImportType
		{
			AssetImportType_Base,
			AssetImportType_Model
		};

		virtual AssetImportType GetType() const { return AssetImportConfig::AssetImportType_Base; }

		bool GenerateThumbnails	= true;		// TODO: Generate a thumbnail for preview in the editor.
	};

	struct ModelImportConfig : public AssetImportConfig
	{
		AssetImportType GetType() const override { return AssetImportConfig::AssetImportType_Model; }

		bool import_skeleton		= false;	// Import Skeleton Bone Structure.
		bool import_animations		= false;	// Import Animations.
		bool import_materials		= true;		// Import embedded materials.
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

		/// @brief The import config for the type of asset being imported
		std::shared_ptr<AssetImportConfig> import_config = nullptr;

		operator bool() const { return handle == NULL_GUID || handle == PLACEHOLDER_0_GUID; } // Indicates Null MetaData
	};
	
	using AssetMap 		= std::unordered_map<AssetHandle, std::shared_ptr<Asset>>;
	using AssetRegistry = std::unordered_map<AssetHandle, AssetMetaData>;

    namespace Util
    {
		static std::string AssetTypeToString(AssetType type)
		{
			switch (type)
			{
				case AssetType::Unknown: return "Unknown";
				case AssetType::PrefabEntity: return "PrefabEntity";
				case AssetType::PrefabModel: return "PrefabModel";
				case AssetType::Texture2D: return "Texture2D";
				case AssetType::TextureCubeMap: return "TextureCubeMap";
				case AssetType::RenderTarget: return "RenderTarget";
				case AssetType::Mesh: return "Mesh";
				case AssetType::Skeleton: return "Skeleton";
				case AssetType::Humanoid: return "Humanoid";
				case AssetType::HumanoidMask: return "HumanoidMask";
				case AssetType::AnimationClip: return "AnimationClip";
				case AssetType::AnimationStateMachine: return "AnimationStateMachine";
				case AssetType::Audio: return "Audio";
				case AssetType::Material: return "Material";
				case AssetType::Skybox: return "Skybox";
				case AssetType::Compute_Shader: return "Compute_Shader";
				case AssetType::Shader: return "Shader";
				case AssetType::PhysicsMaterial: return "PhysicsMaterial";
				default: return "Unknown";
			}
		}
		
		static AssetType AssetTypeFromString(const std::string& str)
		{
			static const std::unordered_map<std::string, AssetType> map = {
				{ "Unknown", AssetType::Unknown },
				{ "PrefabEntity", AssetType::PrefabEntity },
				{ "PrefabModel", AssetType::PrefabModel },
				{ "Texture2D", AssetType::Texture2D },
				{ "TextureCubeMap", AssetType::TextureCubeMap },
				{ "RenderTarget", AssetType::RenderTarget },
				{ "Mesh", AssetType::Mesh },
				{ "Skeleton", AssetType::Skeleton },
				{ "Humanoid", AssetType::Humanoid },
				{ "HumanoidMask", AssetType::HumanoidMask },
				{ "AnimationClip", AssetType::AnimationClip },
				{ "AnimationStateMachine", AssetType::AnimationStateMachine },
				{ "Audio", AssetType::Audio },
				{ "Material", AssetType::Material },
				{ "Skybox", AssetType::Skybox },
				{ "Compute_Shader", AssetType::Compute_Shader },
				{ "Shader", AssetType::Shader },
				{ "PhysicsMaterial", AssetType::PhysicsMaterial }
			};

			auto it = map.find(str);
			if (it != map.end())
				return it->second;
			return AssetType::Unknown;
		}
    }
}