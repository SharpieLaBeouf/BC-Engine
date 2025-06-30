#pragma once

#include "Asset.h"

#include <string>
#include <memory>
#include <unordered_map>

namespace BC
{
    class Shader;

    class AssetManagerBase
    {

	public:

		virtual ~AssetManagerBase() = default;

		const AssetMetaData& GetMetaData(AssetHandle handle);
        const AssetMetaData& GetMetaData(const std::filesystem::path& asset_path);
    
		virtual std::shared_ptr<Asset> GetAsset(const AssetHandle& handle) = 0;
		virtual std::shared_ptr<Shader> GetInbuiltShader(const std::string& default_shader_name, bool is_compute = false) = 0;

		virtual bool IsAssetHandleValid(const AssetHandle& handle) const = 0;
		virtual bool IsAssetLoaded(const AssetHandle& handle) const = 0;
		virtual AssetType GetAssetType(const AssetHandle& handle) const = 0;

		virtual void AddRuntimeAsset(std::shared_ptr<Asset> asset, AssetHandle asset_handle, AssetMetaData asset_meta_data) = 0;
		virtual void RemoveRuntimeAsset(AssetHandle asset_handle) = 0;

		virtual void ClearRuntimeAssets() = 0;

	protected:

		std::unordered_map<AssetHandle, AssetMetaData> m_MetaDataRegistry = {};

    };

	class EditorAssetManager : public AssetManagerBase
	{

	public:

		EditorAssetManager() = default;
		~EditorAssetManager() override = default;
    
		std::shared_ptr<Asset> GetAsset(const AssetHandle& handle) override
		{
			return nullptr;
		}

		std::shared_ptr<Shader> GetInbuiltShader(const std::string& default_shader_name, bool is_compute = false) override
		{
			return nullptr;
		}

		bool IsAssetHandleValid(const AssetHandle& handle) const override
		{
			return false;
		}

		bool IsAssetLoaded(const AssetHandle& handle) const override
		{
			return false;
		}

		AssetType GetAssetType(const AssetHandle& handle) const override
		{
			return AssetType::Unknown;
		}

		void AddRuntimeAsset(std::shared_ptr<Asset> asset, AssetHandle asset_handle, AssetMetaData asset_meta_data) override
		{

		}

		void RemoveRuntimeAsset(AssetHandle asset_handle) override
		{

		}

		void ClearRuntimeAssets() override
		{

		}

	private:

	};

	class RuntimeAssetManager : public AssetManagerBase
	{

	public:

		RuntimeAssetManager() = default;
		~RuntimeAssetManager() override = default;
    
		std::shared_ptr<Asset> GetAsset(const AssetHandle& handle) override
		{
			return nullptr;
		}

		std::shared_ptr<Shader> GetInbuiltShader(const std::string& default_shader_name, bool is_compute = false) override
		{
			return nullptr;
		}

		bool IsAssetHandleValid(const AssetHandle& handle) const override
		{
			return false;
		}

		bool IsAssetLoaded(const AssetHandle& handle) const override
		{
			return false;
		}

		AssetType GetAssetType(const AssetHandle& handle) const override
		{
			return AssetType::Unknown;
		}

		void AddRuntimeAsset(std::shared_ptr<Asset> asset, AssetHandle asset_handle, AssetMetaData asset_meta_data) override
		{

		}

		void RemoveRuntimeAsset(AssetHandle asset_handle) override
		{

		}

		void ClearRuntimeAssets() override
		{

		}

	private:

	};

}