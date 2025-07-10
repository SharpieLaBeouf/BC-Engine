#pragma once

#include "Asset.h"

#include <mutex>
#include <memory>
#include <cstdint>
#include <functional>
#include <filesystem>
#include <unordered_set>

namespace BC
{

    class Material;

    class AssetImporter
    {

    public:

        struct ImportParameters
        {
            AssetHandle asset_handle;
            AssetMetaData& asset_meta_data;

            AssetMap& asset_map;
            AssetRegistry& meta_data_registry;

            std::filesystem::path absolute_asset_directory;
        };

        /// @brief This will load an asset from raw memory data
        /// @param import_parameters This is a struct of information required to load the asset
        /// @param data This is the data in memory to load as a vector of 1 byte's
        /// @return Returns the final loaded asset
        static std::shared_ptr<Asset> ImportAsset(ImportParameters& import_parameters, const std::vector<uint8_t>& data);

        /// @brief This will load a file into memory to be provided for asset import
        /// @param asset_file_path The absolute file path of the asset to load
        /// @return Returns a vector of data in bytes
        static std::vector<uint8_t> LoadDataFromFile(const std::filesystem::path& asset_file_path);

    private:

        // --- Import Prefabs ----
        static std::shared_ptr<Asset> ImportPrefabEntity(ImportParameters& import_parameters, const std::vector<uint8_t>& data);

        // --- Import Models ----
        static std::shared_ptr<Asset> ImportPrefabModel(ImportParameters& import_parameters, const std::vector<uint8_t>& data);

        // --- Import Textures ----
        static std::shared_ptr<Asset> ImportTexture2D(ImportParameters& import_parameters, const std::vector<uint8_t>& data);
        static std::shared_ptr<Asset> ImportTextureCubeMap(ImportParameters& import_parameters, const std::vector<uint8_t>& data);

        // ---- Import Materials / Skybox ----
        static std::shared_ptr<Asset> ImportMaterial(ImportParameters& import_parameters, const std::vector<uint8_t>& data);
        static std::shared_ptr<Asset> ImportSkybox(ImportParameters& import_parameters, const std::vector<uint8_t>& data);

        // ---- Import Shaders ----
        static std::shared_ptr<Asset> ImportShader(ImportParameters& import_parameters, const std::vector<uint8_t>& data);
        static std::shared_ptr<Asset> ImportComputeShader(ImportParameters& import_parameters, const std::vector<uint8_t>& data);

        // ---- Import Animator Machine ----
        static std::shared_ptr<Asset> ImportAnimatorStateMachine(ImportParameters& import_parameters, const std::vector<uint8_t>& data);
        
        // ---- Import Humanoid ----
        static std::shared_ptr<Asset> ImportHumanoid(ImportParameters& import_parameters, const std::vector<uint8_t>& data);
        static std::shared_ptr<Asset> ImportHumanoidMask(ImportParameters& import_parameters, const std::vector<uint8_t>& data);

        // ---- Import Physics Material ---
        static std::shared_ptr<Asset> ImportPhysicsMaterial(ImportParameters& import_parameters, const std::vector<uint8_t>& data);

    private:

	    using AssetImportFunction = std::function<std::shared_ptr<Asset>(ImportParameters&, const std::vector<uint8_t>&)>;
        static std::unordered_map<AssetType, AssetImportFunction> s_AssetImportFunctions;
    
        static std::mutex s_AssetsLoadingMutex;
        static std::unordered_set<AssetHandle> s_AssetsLoading;

    };

}