#pragma once

// Core Headers
#include "AssetManager.h"
#include "Core/Application.h"

// C++ Standard Library Headers
#include <cstdint>
#include <memory>

// External Vendor Library Headers

namespace BC
{
    namespace Util
    {

    }

    class AssetManager
    {

    public:

		static AssetType GetAssetTypeFromFileExtension(const std::filesystem::path& extension);
		static bool IsExtensionSupported(const std::filesystem::path& extension);
		static bool IsAssetTypeComposite(const AssetType& asset_type);

		static bool IsAssetLoaded(AssetHandle handle)
		{
			return Application::GetProject()->GetAssetManager()->IsAssetLoaded(handle);
		}

        template <typename T>
        static std::shared_ptr<T> GetAsset(AssetHandle asset_handle)
        {
            std::shared_ptr<Asset> asset = Application::GetProject()->GetAssetManager()->GetAsset(asset_handle);
            return std::static_pointer_cast<T>(asset);
        }

        static bool IsAssetHandleValid(AssetHandle asset_handle)
        {
            return Application::GetProject()->GetAssetManager()->IsAssetHandleValid(asset_handle);
        }

        static void RemoveRuntimeAsset(AssetHandle asset_handle)
        {
            Application::GetProject()->GetAssetManager()->RemoveRuntimeAsset(asset_handle);
        }

        static void ClearRuntimeAssets()
        {
            Application::GetProject()->GetAssetManager()->ClearRuntimeAssets();
        }

    };

}