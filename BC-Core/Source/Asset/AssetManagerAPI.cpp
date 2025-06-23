#include "BC_PCH.h"
#include "AssetManagerAPI.h"

namespace BC
{
    
	static std::map<std::filesystem::path, AssetType> s_AssetExtensionMap = {

		{ ".scene",	            AssetType::Scene },
		{ ".prefab",	        AssetType::Prefab },

		{ ".png",		        AssetType::Texture2D },
		{ ".jpg",		        AssetType::Texture2D },
		{ ".jpeg",		        AssetType::Texture2D },
		{ ".psd",		        AssetType::Texture2D },
		{ ".tga",		        AssetType::Texture2D },
		{ ".tif",		        AssetType::Texture2D },

		{ ".obj",		        AssetType::ModelImport },
		{ ".fbx",		        AssetType::ModelImport },

		{ ".animator",          AssetType::AnimationStateMachine },

		{ ".mp3",		        AssetType::Audio },

		{ ".material",	        AssetType::Material_Standard },
		{ ".skybox",	        AssetType::Material_Skybox },

		{ ".comp",		        AssetType::Compute_Shader },
		{ ".compute",	        AssetType::Compute_Shader },
		{ ".glsl",		        AssetType::Shader },
		{ ".shader",	        AssetType::Shader },

		{ ".humanoid",		    AssetType::Humanoid },
		{ ".humanoidmask",	    AssetType::HumanoidMask }

	};

	AssetType AssetManager::GetAssetTypeFromFileExtension(const std::filesystem::path& extension)
	{
		if (s_AssetExtensionMap.find(extension) == s_AssetExtensionMap.end())
		{
			BC_CORE_WARN("Could not find AssetType for {0}", extension.string().c_str());
			return AssetType::Unknown;
		}

		return s_AssetExtensionMap.at(extension);
	}

	bool AssetManager::IsExtensionSupported(const std::filesystem::path& extension) 
    {
		if (s_AssetExtensionMap.find(extension) == s_AssetExtensionMap.end())
			return false;
		return true;
	}

	bool AssetManager::IsAssetTypeComposite(const AssetType& asset_type)
	{
		switch (asset_type) 
        {
			case AssetType::Scene:
			case AssetType::Prefab:
			case AssetType::ModelImport:
			case AssetType::Material_Skybox:
			case AssetType::Material_Standard:
			case AssetType::AnimationStateMachine:
			{
				return true;
			}
		}
		return false;
	}
}
