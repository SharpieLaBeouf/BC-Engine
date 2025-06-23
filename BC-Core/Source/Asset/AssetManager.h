#pragma once

#include "Asset.h"

#include <string>
#include <memory>

namespace BC
{
    class Shader;

    class AssetManagerBase
    {

	public:

		virtual ~AssetManagerBase() = default;
    
		virtual std::shared_ptr<Asset> GetAsset(const AssetHandle& handle) = 0;
		virtual std::shared_ptr<Shader> GetInbuiltShader(const std::string& default_shader_name, bool is_compute = false) = 0;

		virtual bool IsAssetHandleValid(const AssetHandle& handle) const = 0;
		virtual bool IsAssetLoaded(const AssetHandle& handle) const = 0;
		virtual AssetType GetAssetType(const AssetHandle& handle) const = 0;

		virtual void AddRuntimeAsset(std::shared_ptr<Asset> asset, AssetHandle asset_handle, AssetMetaData asset_meta_data) = 0;
		virtual void RemoveRuntimeAsset(AssetHandle asset_handle) = 0;

		virtual void ClearRuntimeAssets() = 0;

    };

	class EditorAssetManager : public AssetManagerBase
	{

	public:
    
		std::shared_ptr<Asset> GetAsset(const AssetHandle& handle)
		{
			return nullptr;
		}

		std::shared_ptr<Shader> GetInbuiltShader(const std::string& default_shader_name, bool is_compute = false)
		{
			return nullptr;
		}

		bool IsAssetHandleValid(const AssetHandle& handle)
		{
			return false;
		}

		bool IsAssetLoaded(const AssetHandle& handle)
		{
			return false;
		}

		AssetType GetAssetType(const AssetHandle& handle)
		{
			return AssetType::Unknown;
		}

		void AddRuntimeAsset(std::shared_ptr<Asset> asset, AssetHandle asset_handle, AssetMetaData asset_meta_data)
		{

		}

		void RemoveRuntimeAsset(AssetHandle asset_handle)
		{

		}

		void ClearRuntimeAssets()
		{

		}


	private:

	};
}