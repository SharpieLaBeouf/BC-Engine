#include "BC_PCH.h"

#include "Asset.h"
#include "AssetImporter.h"
#include "AssetManagerAPI.h"

#include "Asset/Assets/AnimationClip.h"
#include "Asset/Assets/AnimationStateMachine/AnimationStateMachine.h"
#include "Asset/Assets/Humanoid.h"
#include "Asset/Assets/Material.h"
#include "Asset/Assets/Prefab.h"
#include "Asset/Assets/Shader.h"
#include "Asset/Assets/Skeleton.h"
#include "Asset/Assets/StaticMesh.h"
#include "Asset/Assets/Texture.h"

#include "Physics/PhysicsWrapper.h"

#include "Util/FileUtil.h"
#include "Util/Hash.h"

#include <yaml-cpp/yaml.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <stb_image.h>

namespace BC
{	
	std::unordered_map<AssetType, AssetImporter::AssetImportFunction> AssetImporter::s_AssetImportFunctions = 
    {

		{ AssetType::PrefabEntity,              ImportPrefabEntity },
		{ AssetType::PrefabModel,				ImportPrefabModel },

		{ AssetType::Texture2D,					ImportTexture2D },
		{ AssetType::TextureCubeMap,			ImportTextureCubeMap },

		{ AssetType::Material,			        ImportMaterial },
		{ AssetType::Skybox,			        ImportSkybox },

		{ AssetType::Shader,					ImportShader },
		{ AssetType::Compute_Shader,			ImportComputeShader },
		
		{ AssetType::AnimationStateMachine,		ImportAnimatorStateMachine },

		{ AssetType::Humanoid,					ImportHumanoid },
		{ AssetType::HumanoidMask,				ImportHumanoidMask },

		{ AssetType::PhysicsMaterial,			ImportPhysicsMaterial },
	};

    std::shared_ptr<Asset> AssetImporter::ImportAsset(ImportParameters& import_parameters, const std::vector<uint8_t>& data)
    {
        if (data.empty() || 
            import_parameters.asset_handle == NULL_GUID || 
            import_parameters.asset_meta_data.type == AssetType::Unknown ||
            s_AssetsLoading.contains(import_parameters.asset_handle))
            return nullptr;
        
        auto it = s_AssetImportFunctions.find(import_parameters.asset_meta_data.type);
        if (it == s_AssetImportFunctions.end())
            return nullptr;

        s_AssetsLoading.insert(import_parameters.asset_handle);
        auto asset = it->second(import_parameters, data);
        s_AssetsLoading.erase(import_parameters.asset_handle);
        
        return asset;
    }

#pragma region PrefabEntity Import

    std::shared_ptr<Asset> AssetImporter::ImportPrefabEntity(ImportParameters& import_parameters, const std::vector<uint8_t>& data)
    {
        std::shared_ptr<Prefab> prefab_entity;
        BC_CATCH_BEGIN();

        // Cast to const char* to std::string to ensure null termination at correct length
        prefab_entity = Prefab::CreatePrefabFromSerialisedData(std::string(reinterpret_cast<const char*>(data.data()), data.size()));

        BC_CATCH_END_RETURN(nullptr);
        return prefab_entity;
    }

#pragma endregion

#pragma region PrefabModel Import

    namespace Util::ModelImport
    {
        // Used to Identify Duplicate Meshes
        struct MeshInstanceKey
		{
			std::vector<uint32_t> mesh_references;
			bool operator==(const MeshInstanceKey& other) const { return mesh_references == other.mesh_references; }
			void normalize() { std::sort(mesh_references.begin(), mesh_references.end()); }

			void GenerateKey(const aiNode* ai_node)
			{
				mesh_references.reserve(ai_node->mNumMeshes);
				for (unsigned int i = 0; i < ai_node->mNumMeshes; i++)
					mesh_references.emplace_back(ai_node->mMeshes[i]);
				normalize();
			}
		};

        // Used to Identify Duplicate Meshes
        struct MeshInstanceKeyHash
		{
			std::size_t operator()(const MeshInstanceKey& node) const
			{
				std::size_t seed = node.mesh_references.size();
				for (uint32_t mesh : node.mesh_references) 
                    seed ^= std::hash<uint32_t>{}(mesh)+0x9e3779b9 + (seed << 6) + (seed >> 2);
				return seed;
			}
		};

        // Info for particular import
        thread_local struct
        {
            Assimp::Importer ai_importer = {};

            bool import_skeleton = false;
            bool import_animations = false;
            bool import_materials = false;
            AssetHandle imported_skeleton_handle = NULL_GUID;

            // Key = MeshInstanceKey
            // Value = Pair of ( StaticMesh handle, and Vector of Material Assets )
            std::unordered_map<MeshInstanceKey, std::pair<AssetHandle, std::vector<AssetHandle>>, MeshInstanceKeyHash> duplicate_nodes = {};

        } s_ImportInfo = {};

        namespace Helpers
        {
            static inline glm::mat4 AssimpMat4ToGLM(const aiMatrix4x4& from)
            {
                glm::mat4 to{};
                //the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
                to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
                to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
                to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
                to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
                return to;
            }

            static inline glm::vec3 AssimpVec3ToGLM(const aiVector3D& vec)
            {
                return glm::vec3(vec.x, vec.y, vec.z);
            }

            static inline glm::quat AssimpQuatToGLM(const aiQuaternion& pOrientation)
            {
                return glm::quat(pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z);
            }

            static inline std::filesystem::path ResolveAssimpTexturePath(const aiString& assimp_texture_string, AssetImporter::ImportParameters& import_parameters)
            {
                std::string assimp_texture_rel = assimp_texture_string.C_Str();
                std::filesystem::path model_path = import_parameters.absolute_asset_directory / import_parameters.asset_meta_data.asset_path;
                std::filesystem::path model_directory = model_path.parent_path();

                std::filesystem::path texture_path = model_directory / assimp_texture_rel;
                texture_path = std::filesystem::weakly_canonical(texture_path);

                // Make relative to Assets folder
                std::filesystem::path relative_texture_path = std::filesystem::relative(texture_path, import_parameters.absolute_asset_directory);

                return Util::NormaliseFilePathToString(relative_texture_path);
            }
        
            static inline vk::Format ParseAssimpFormatHint(const aiTexture* embedded_texture)
            {
                std::string hint = embedded_texture->achFormatHint;

                if (hint.empty())
                    return vk::Format::eUndefined;

                if (hint == "rgba8888") return vk::Format::eR8G8B8A8Unorm;
                if (hint == "bgra8888") return vk::Format::eB8G8R8A8Unorm;
                if (hint == "argb8888") return vk::Format::eR8G8B8A8Unorm;

                if (hint == "rgb888")   return vk::Format::eR8G8B8Unorm;
                if (hint == "bgr888")   return vk::Format::eB8G8R8Unorm;
                if (hint == "rg88")     return vk::Format::eR8G8Unorm;
                if (hint == "r8")       return vk::Format::eR8Unorm;

                if (hint == "rgba4444") return vk::Format::eR4G4B4A4UnormPack16;
                if (hint == "rgba5551") return vk::Format::eR5G5B5A1UnormPack16;
                if (hint == "argb1555") return vk::Format::eB5G5R5A1UnormPack16;
                if (hint == "rgba5650") return vk::Format::eR5G6B5UnormPack16;

                if (hint == "rgba1010102") return vk::Format::eA2B10G10R10UnormPack32;

                BC_CORE_WARN("ParseAssimpFormatHint: Unknown embedded texture format hint: '{}'", hint);
                return vk::Format::eUndefined;
            }
        }

        static void ProcessAnimations(const aiScene* model_scene, AssetImporter::ImportParameters& import_parameters)
        {

        }

        static void ProcessMesh(const aiScene* model_scene, const aiMesh* mesh, std::shared_ptr<StaticMesh>& mesh_asset)
        {

        }

        static void ProcessAssimpTexture(const aiScene* model_scene, const aiMaterial* material, aiTextureType texture_type, std::shared_ptr<Material>& material_asset, AssetImporter::ImportParameters& import_parameters)
        {
            if (texture_type == aiTextureType_NONE || material->GetTextureCount(texture_type) == 0)
                return;

            aiString assimp_texture_string;
            material->GetTexture(texture_type, 0, &assimp_texture_string);
            
            AssetHandle texture_handle = NULL_GUID;
            std::shared_ptr<Asset> texture_asset = nullptr;

            // If the Assimp Texture is Embedded in Model File
            bool is_embedded = assimp_texture_string.length > 0 && assimp_texture_string.C_Str()[0] == '*';
            if (is_embedded)
            {
                int embedded_index = std::atoi(assimp_texture_string.C_Str() + 1);
                aiTexture* embedded_texture = model_scene->mTextures[embedded_index];
                if (!embedded_texture)
                {
                    BC_CORE_WARN("ProcessAssimpTexture: Could Not Import Embedded Texture - '{}'.", assimp_texture_string.C_Str());
                    return;
                }

                texture_handle = Util::HashString
                (
                    Util::AssetTypeToString(AssetType::Texture2D) +
					Util::NormaliseFilePathToString(import_parameters.asset_meta_data.asset_path) + 
					embedded_texture->mFilename.C_Str()
                );

                if (!import_parameters.asset_map.contains(texture_handle))
                {
                    AssetMetaData embedded_texture_metadata;
                    embedded_texture_metadata.name = std::filesystem::path(embedded_texture->mFilename.C_Str()).stem().string();
                    embedded_texture_metadata.asset_path = import_parameters.asset_meta_data.asset_path;
                    embedded_texture_metadata.type = AssetType::Texture2D;
                    embedded_texture_metadata.handle = texture_handle;
                    embedded_texture_metadata.parent_handle = import_parameters.asset_handle;
                    embedded_texture_metadata.has_children_assets = false;
                    embedded_texture_metadata.runtime_asset = import_parameters.asset_meta_data.runtime_asset;
                    embedded_texture_metadata.inbuilt_asset = import_parameters.asset_meta_data.inbuilt_asset;

                    if (embedded_texture->mHeight == 0)
                    {
                        // Compressed Texture

                        // Pass compressed texture file data to AssetImporter
                        AssetImporter::ImportParameters import_params = 
                        {
                            .asset_handle = texture_handle,
                            .asset_meta_data = embedded_texture_metadata,
                            .asset_map = import_parameters.asset_map,
                            .meta_data_registry = import_parameters.meta_data_registry,
                            .absolute_asset_directory = import_parameters.absolute_asset_directory
                        };

                        const uint8_t* data_ptr = reinterpret_cast<const uint8_t*>(embedded_texture->pcData);
                        size_t data_size = static_cast<size_t>(embedded_texture->mWidth);

                        std::vector<uint8_t> raw_compressed_data(data_ptr, data_ptr + data_size);

                        texture_asset = AssetImporter::ImportAsset(import_params, raw_compressed_data);
                        if (texture_asset && texture_asset->GetType() == AssetType::Texture2D)
                        {
                            // Ensure is placed in registry
                            import_parameters.asset_map[texture_handle] = texture_asset;
                            import_parameters.meta_data_registry[texture_handle] = embedded_texture_metadata;
                        }
                    }
                    else
                    {
                        // Interpet format type
                        
                        Texture2DSpecification specification = {};
                        specification.width = embedded_texture->mWidth;
                        specification.height = embedded_texture->mHeight;
                        specification.format = Helpers::ParseAssimpFormatHint(embedded_texture);

                        if (specification.format == vk::Format::eUndefined)
                        {
                            BC_CORE_WARN("ProcessAssimpTexture: Unsupported embedded texture format hint: '{}'.", embedded_texture->achFormatHint);
                            return;
                        }

                        specification.generate_mips = true;
                        specification.mip_levels = std::max(specification.width, specification.height);
                        specification.mip_levels = specification.mip_levels > 1
                            ? static_cast<uint32_t>(std::floor(std::log2(specification.mip_levels))) + 1
                            : 1;
                        specification.mip_levels = std::min(specification.mip_levels, 5u);

                        texture_asset = Texture2D::CreateTexture(specification, reinterpret_cast<unsigned char*>(embedded_texture->pcData), specification.format);
                        if (texture_asset && texture_asset->GetType() == AssetType::Texture2D)
                        {
                            // Ensure is placed in registry
                            import_parameters.asset_map[texture_handle] = texture_asset;
                            import_parameters.meta_data_registry[texture_handle] = embedded_texture_metadata;
                        }
                    }
                }
                else
                {
                    texture_asset = import_parameters.asset_map.at(texture_handle);
                }
            }
            else
            {
                std::filesystem::path texture_file_path = Helpers::ResolveAssimpTexturePath(assimp_texture_string, import_parameters);
                
                if (!std::filesystem::exists(import_parameters.absolute_asset_directory / texture_file_path))
                {
                    BC_CORE_WARN("Could Not Get Texture Path for Texture.");
                }

                texture_handle = Util::HashString
                (
                    Util::AssetTypeToString(AssetType::Texture2D) +
                    Util::NormaliseFilePathToString(texture_file_path)
                );

                // Import Texture

                if (import_parameters.asset_map.contains(texture_handle)) // Loaded Already
                {
                    texture_asset = import_parameters.asset_map.at(texture_handle);
                }
                else if (import_parameters.meta_data_registry.contains(texture_handle)) // Valid but not loaded
                {
                    texture_asset = AssetManager::GetAsset<Texture2D>(texture_handle); // Force Load
                }
                else // Needs to be imported for first time
                {
                    AssetMetaData texture_metadata;
                    texture_metadata.name = texture_file_path.stem().string();
                    texture_metadata.asset_path = texture_file_path;
                    texture_metadata.type = AssetType::Texture2D;
                    texture_metadata.handle = texture_handle;
                    texture_metadata.parent_handle = NULL_GUID;
                    texture_metadata.has_children_assets = false;
                    texture_metadata.runtime_asset = import_parameters.asset_meta_data.runtime_asset;
                    texture_metadata.inbuilt_asset = import_parameters.asset_meta_data.inbuilt_asset;

                    AssetImporter::ImportParameters import_params = 
                    {
                        .asset_handle = texture_handle,
                        .asset_meta_data = texture_metadata,
                        .asset_map = import_parameters.asset_map,
                        .meta_data_registry = import_parameters.meta_data_registry,
                        .absolute_asset_directory = import_parameters.absolute_asset_directory
                    };

                    texture_asset = AssetImporter::ImportAsset(import_params, Util::LoadDataStreamFromFile(import_parameters.absolute_asset_directory / texture_file_path));
                    if (texture_asset && texture_asset->GetType() == AssetType::Texture2D)
                    {
                        // Ensure is placed in registry
                        import_parameters.asset_map[texture_handle] = texture_asset;
                        import_parameters.meta_data_registry[texture_handle] = texture_metadata;
                    }
                }
            }

            if (!texture_asset)
                return;

            switch(texture_type)
            {
                case aiTextureType_BASE_COLOR:
                case aiTextureType_DIFFUSE:
                {
                    material_asset->SetAlbedoTexture(texture_handle);
                    break;
                }
                case aiTextureType_METALNESS:
                {
                    material_asset->SetMetallicTexture(texture_handle);
                    break;
                }
                case aiTextureType_NORMALS:
                {
                    material_asset->SetNormalTexture(texture_handle);
                    break;
                }
            }
        }

        static AssetHandle ProcessMaterial(const aiScene* model_scene, const aiMesh* mesh, AssetImporter::ImportParameters& import_parameters)
        {
            if (!s_ImportInfo.import_materials)
                return NULL_GUID;

            AssetHandle material_handle;

            const aiMaterial* material = model_scene->mMaterials[mesh->mMaterialIndex];
            if (!material)
            {
                return Util::HashString
                (
                    Util::AssetTypeToString(AssetType::Material) +
                    "InBuiltAsset_DefaultMaterial"
                );
            }

            aiString material_name;
            material->Get(AI_MATKEY_NAME, material_name);

            // Determine Material Handle
            if (material_name.length == 0 || material_name == aiString("DefaultMaterial"))
            {
                return Util::HashString
                (
                    Util::AssetTypeToString(AssetType::Material) +
                    "InBuiltAsset_DefaultMaterial"
                );
            }
            else
            {
                material_handle = Util::HashString
                (
                    Util::AssetTypeToString(AssetType::Material) +
					Util::NormaliseFilePathToString(import_parameters.asset_meta_data.asset_path) + 
					material_name.C_Str()
                );
            }

            // Material Already Loaded -> Early Exit
            if (import_parameters.meta_data_registry.contains(material_handle))
            {
                return material_handle;
            }

            AssetMetaData material_metadata;
            material_metadata.name = material_name.C_Str();
            material_metadata.asset_path = import_parameters.asset_meta_data.asset_path;
            material_metadata.type = AssetType::Material;
            material_metadata.handle = material_handle;
            material_metadata.parent_handle = import_parameters.asset_handle;
            material_metadata.has_children_assets = true; // Materials Have Children Assets -> textures
            material_metadata.runtime_asset = import_parameters.asset_meta_data.runtime_asset;
            material_metadata.inbuilt_asset = import_parameters.asset_meta_data.inbuilt_asset;
            
			std::shared_ptr<Material> material_asset = std::make_shared<Material>();
            material_asset->Handle = material_handle;

			ai_real value = 0.0f;
			aiColor4D colour = { 1.0f, 1.0f, 1.0f, 1.0f };

            // Deet Albedo Tint Colour
			if (material->Get(AI_MATKEY_BASE_COLOR, colour) == aiReturn_SUCCESS)
            {
				material_asset->SetAlbedoColour({ colour.r, colour.g, colour.b, colour.a });
            }
			else if (material->Get(AI_MATKEY_COLOR_DIFFUSE, colour) == aiReturn_SUCCESS)
            {
				material_asset->SetAlbedoColour({ colour.r, colour.g, colour.b, colour.a });
            }
            else
            {
                material_asset->SetAlbedoColour({ 1.0f, 1.0f, 1.0f, 1.0f });
            }

			if (material->Get(AI_MATKEY_METALLIC_FACTOR, value) == aiReturn_SUCCESS)
            {
				material_asset->SetMetallic(value);
            }

			if (material->Get(AI_MATKEY_ROUGHNESS_FACTOR, value) == aiReturn_SUCCESS)
            {
				material_asset->SetRoughness(value);
            }

			if (material->Get(AI_MATKEY_TRANSPARENCYFACTOR, value) == aiReturn_SUCCESS && value > 0.0f)
			{
				material_asset->SetAlpha(1.0f - value);
			}
            
			aiTextureType texture_base_colour_type = material->GetTextureCount(aiTextureType_BASE_COLOR) > 0 ? aiTextureType_BASE_COLOR : material->GetTextureCount(aiTextureType_DIFFUSE) > 0 ? aiTextureType_DIFFUSE : aiTextureType_NONE;

			ProcessAssimpTexture(model_scene, material, texture_base_colour_type, material_asset, import_parameters);
			ProcessAssimpTexture(model_scene, material, aiTextureType_METALNESS, material_asset, import_parameters);
			ProcessAssimpTexture(model_scene, material, aiTextureType_NORMALS, material_asset, import_parameters);

            import_parameters.asset_map[material_handle] = material_asset;
            import_parameters.meta_data_registry[material_handle] = material_metadata;

            return material_handle;
        }

        static void ProcessStaticNode(const aiScene* model_scene, aiNode* current_node, PrefabEntity& current_node_entity, Prefab& model_prefab_asset, AssetImporter::ImportParameters& import_parameters)
        {
            if (!model_scene || !current_node || !current_node_entity)
                return;
            
            // Only set local transform matrix if not the root node. Root node must always be at 0,0,0.
            if (current_node != model_scene->mRootNode)
            {
                current_node_entity.GetTransform().SetMatrix(Helpers::AssimpMat4ToGLM(current_node->mTransformation), false, false);
            }

			MeshInstanceKey node_key{};
			node_key.GenerateKey(current_node);

            if (!s_ImportInfo.duplicate_nodes.contains(node_key))
            {
				AssetHandle mesh_handle = Util::HashString
                (
					Util::AssetTypeToString(AssetType::Mesh) + 
					Util::NormaliseFilePathToString(import_parameters.asset_meta_data.asset_path) + 
					current_node->mName.C_Str()
				);

				std::shared_ptr<StaticMesh> mesh_asset = std::make_shared<StaticMesh>();
				mesh_asset->Handle = mesh_handle;

				AssetMetaData mesh_metadata;
				mesh_metadata.name = current_node->mName.C_Str();
				mesh_metadata.asset_path = import_parameters.asset_meta_data.asset_path;
				mesh_metadata.type = AssetType::Mesh;
				mesh_metadata.handle = mesh_handle;
				mesh_metadata.parent_handle = import_parameters.asset_handle;
				mesh_metadata.has_children_assets = false; // Meshes won't have children assets
				mesh_metadata.runtime_asset = import_parameters.asset_meta_data.runtime_asset;
				mesh_metadata.inbuilt_asset = import_parameters.asset_meta_data.inbuilt_asset;

                auto& mesh_renderer_component = current_node_entity.AddComponent<MeshRendererComponent>();
                mesh_renderer_component.SetMesh(mesh_handle);

                for (uint32_t mesh_index = 0; mesh_index < current_node->mNumMeshes; ++mesh_index)
                {
                    const aiMesh* mesh = model_scene->mMeshes[current_node->mMeshes[mesh_index]];
                    if (!mesh)
                        continue;

                    ProcessMesh(model_scene, mesh, mesh_asset);
                    ProcessMaterial(model_scene, mesh, import_parameters);
                }

                auto& loaded_mesh = s_ImportInfo.duplicate_nodes[node_key];
                loaded_mesh.first = mesh_renderer_component.GetMesh();
                loaded_mesh.second = mesh_renderer_component.GetMaterialHandles();
            }
            else
            {
                // Duplicate aiNode already loaded, can just replicate into this entity
				auto& mesh_renderer_component = current_node_entity.AddComponent<MeshRendererComponent>();
                mesh_renderer_component.SetMesh(s_ImportInfo.duplicate_nodes[node_key].first);
                mesh_renderer_component.SetMaterials(s_ImportInfo.duplicate_nodes[node_key].second);
            }

            for (uint32_t i = 0; i < current_node->mNumChildren; i++)
            {
                PrefabEntity child_entity = model_prefab_asset.CreateEntity(current_node->mChildren[i]->mName.C_Str());
                child_entity.AttachParent(current_node_entity);

                ProcessStaticNode(model_scene, current_node->mChildren[i], child_entity, model_prefab_asset, import_parameters);
            }
        }

        static PrefabEntity ProcessSkeleton(const aiScene* model_scene, aiNode* current_node, AssetImporter::ImportParameters& import_parameters)
        {

        }

        static void ProcessSkinnedNode(const aiScene* model_scene, aiNode* current_node, PrefabEntity& current_node_entity, Prefab& model_prefab_asset, AssetImporter::ImportParameters& import_parameters)
        {

            
            for (uint32_t i = 0; i < current_node->mNumChildren; i++)
            {
                PrefabEntity child_entity = model_prefab_asset.CreateEntity(current_node->mChildren[i]->mName.C_Str());
                child_entity.AttachParent(current_node_entity);

                ProcessSkinnedNode(model_scene, current_node->mChildren[i], child_entity, model_prefab_asset, import_parameters);
            }
        }
    }

    std::shared_ptr<Asset> AssetImporter::ImportPrefabModel(ImportParameters& import_parameters, const std::vector<uint8_t>& data)
    {
        const aiScene* model_scene = Util::ModelImport::s_ImportInfo.ai_importer.ReadFileFromMemory
        (
            data.data(), 
            data.size(), 

			aiProcess_Triangulate |
			aiProcess_SortByPType |
			aiProcess_SplitLargeMeshes |
			aiProcess_ImproveCacheLocality |
			aiProcess_JoinIdenticalVertices |
			aiProcess_RemoveRedundantMaterials |

			aiProcess_GenUVCoords |
			aiProcess_GenSmoothNormals |
			aiProcess_GenBoundingBoxes |

			aiProcess_CalcTangentSpace |
			aiProcess_LimitBoneWeights |
			aiProcess_PopulateArmatureData |

			aiProcess_FindInstances |
			aiProcess_FindDegenerates |
			aiProcess_FindInvalidData
        );

        if (!model_scene)
        {
            BC_CORE_ERROR("AssetImporter::ImportPrefabModel: Failed to Import Model: AssetName - '{}', Error String - '{}'.", import_parameters.asset_meta_data.name, Util::ModelImport::s_ImportInfo.ai_importer.GetErrorString());
            Util::ModelImport::s_ImportInfo.ai_importer.FreeScene();
            return nullptr;
        }
        
        if (!(model_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) && !model_scene->mRootNode)
        {
            BC_CORE_ERROR("AssetImporter::ImportPrefabModel: Imported Model is Missing a Root Node or is Marked as Incomplete: AssetName - '{}'.", import_parameters.asset_meta_data.name);
            Util::ModelImport::s_ImportInfo.ai_importer.FreeScene();
            return nullptr;
        }

        ModelImportConfig* import_config = reinterpret_cast<ModelImportConfig*>(import_parameters.asset_meta_data.import_config.get());
        Util::ModelImport::s_ImportInfo.import_skeleton     = import_config->import_skeleton;
        Util::ModelImport::s_ImportInfo.import_animations   = import_config->import_animations;
        Util::ModelImport::s_ImportInfo.import_materials    = import_config->import_materials;

        // Create Prefab
        std::shared_ptr<Prefab> prefab_model_asset = std::make_shared<Prefab>();
        prefab_model_asset->SetPrefabType(PrefabType_Model); // Set Type to Model
        prefab_model_asset->SetName(import_parameters.asset_meta_data.name); // Set Prefab Name to Asset Name

        // Create Root Entity
        PrefabEntity root_entity = prefab_model_asset->CreateEntity(import_parameters.asset_meta_data.name);
        prefab_model_asset->SetRootEntity(root_entity);

        if (model_scene->mRootNode->mNumChildren == 1)
        {
            if (Util::ModelImport::s_ImportInfo.import_skeleton)
            {
                Util::ModelImport::ProcessSkinnedNode(model_scene, model_scene->mRootNode, root_entity, *prefab_model_asset, import_parameters);
            }
            else
            {
                Util::ModelImport::ProcessStaticNode(model_scene, model_scene->mRootNode, root_entity, *prefab_model_asset, import_parameters);
            }
        }
        else if (model_scene->mRootNode->mNumChildren > 1)
        {
            for (uint32_t i = 0; i < model_scene->mRootNode->mNumChildren; ++i)
            {
                PrefabEntity child_entity = prefab_model_asset->CreateEntity(model_scene->mRootNode->mChildren[i]->mName.C_Str());
                child_entity.AttachParent(root_entity);
                
                if (Util::ModelImport::s_ImportInfo.import_skeleton)
                {
                    Util::ModelImport::ProcessSkinnedNode(model_scene, model_scene->mRootNode->mChildren[i], child_entity, *prefab_model_asset, import_parameters);
                }
                else
                {
                    Util::ModelImport::ProcessStaticNode(model_scene, model_scene->mRootNode->mChildren[i], child_entity, *prefab_model_asset, import_parameters);
                }
            }
        }
        else
        {
            Util::ModelImport::s_ImportInfo.ai_importer.FreeScene();
            Util::ModelImport::s_ImportInfo.import_skeleton = false;
            Util::ModelImport::s_ImportInfo.import_animations = false;
            Util::ModelImport::s_ImportInfo.import_materials = false;
            Util::ModelImport::s_ImportInfo.imported_skeleton_handle = NULL_GUID;
            Util::ModelImport::s_ImportInfo.duplicate_nodes.clear();
            return nullptr;
        }

        if (Util::ModelImport::s_ImportInfo.import_skeleton)
        {
            auto view = prefab_model_asset->GetAllEntitiesWith<SkinnedMeshRendererComponent>();
            for (auto& prefab_entity_handle : view)
            {
                PrefabEntity prefab_entity = { prefab_entity_handle, prefab_model_asset.get() };
                if (!prefab_entity)
                    continue;

                SkinnedMeshRendererComponent* component = nullptr;
                if (component = prefab_entity.TryGetComponent<SkinnedMeshRendererComponent>(); !component)
                    continue;
                
                component->CalculatePrefabBindPoseBoneTransformations(prefab_model_asset);
            }
        }

        if (Util::ModelImport::s_ImportInfo.import_animations)
        {
            Util::ModelImport::ProcessAnimations(model_scene, import_parameters);
        }
        
        // TODO: Implement Model Loading -> Prefab of type PrefabModel
        // Load Meshes
        // Load Textures    -> DONE to test
        // Load Materials   -> DONE to test
        // Load Skeleton
        // Load Animations
        
        // Create Prefab Hierarchy -> this will be Prefab of type PrefabModel
        // which indicates its source cannot be modified, but it can be
        // serialised into it's own PrefabEntity. This would still require the
        // PrefabModel to be laoded to access all the data of the children
        // assets, e.g., meshes, animations, etc.
        
        // E.g., you can load the model -> instantiate in scene. When
        // deserliased from scene into a PrefabEntity the hierarchy and
        // compoennts can be changed, but he underlying data sources still point
        // to the parent PrefabModel including materials, textures, meshes,
        // animations. This means the PrefabModel will ALWAYS need to be in the
        // scene if you are to successfully use the assets its newly created
        // PrefabEntity point to.
        
        Util::ModelImport::s_ImportInfo.ai_importer.FreeScene();
        Util::ModelImport::s_ImportInfo.import_skeleton = false;
        Util::ModelImport::s_ImportInfo.import_animations = false;
        Util::ModelImport::s_ImportInfo.import_materials = false;
        Util::ModelImport::s_ImportInfo.imported_skeleton_handle = NULL_GUID;
        Util::ModelImport::s_ImportInfo.duplicate_nodes.clear();
        return nullptr;
    }

#pragma endregion

#pragma region Texture2D Import

    std::shared_ptr<Asset> AssetImporter::ImportTexture2D(ImportParameters& import_parameters, const std::vector<uint8_t>& data)
    {
        return Texture2D::CreateTextureFromFileBytes(data);
    }

    std::shared_ptr<Asset> AssetImporter::ImportTextureCubeMap(ImportParameters& import_parameters, const std::vector<uint8_t>& data)
    {
        std::shared_ptr<TextureCubeMap> cube_map;
        BC_CATCH_BEGIN();

        // Cast to const char* to std::string to ensure null termination at correct length
        cube_map = TextureCubeMap::CreateTextureCubeMap(std::string(reinterpret_cast<const char*>(data.data()), data.size()));

        BC_CATCH_END_RETURN(nullptr);
        return cube_map;
    }

#pragma endregion

#pragma region Material Import

    std::shared_ptr<Asset> AssetImporter::ImportMaterial(ImportParameters& import_parameters, const std::vector<uint8_t>& data)
    {
        std::shared_ptr<Material> material;
        BC_CATCH_BEGIN();

        // Cast to const char* to std::string to ensure null termination at correct length
        material = Material::CreateMaterial(std::string(reinterpret_cast<const char*>(data.data()), data.size()));

        BC_CATCH_END_RETURN(nullptr);
        return material;
    }

    std::shared_ptr<Asset> AssetImporter::ImportSkybox(ImportParameters& import_parameters, const std::vector<uint8_t>& data)
    {
        std::shared_ptr<Skybox> skybox;
        BC_CATCH_BEGIN();

        // Cast to const char* to std::string to ensure null termination at correct length
        skybox = Skybox::CreateSkybox(std::string(reinterpret_cast<const char*>(data.data()), data.size()));

        BC_CATCH_END_RETURN(nullptr);
        return skybox;
    }

#pragma endregion

#pragma region Shader Import

    std::shared_ptr<Asset> AssetImporter::ImportShader(ImportParameters& import_parameters, const std::vector<uint8_t>& data)
    {
        std::shared_ptr<Shader> shader;
        BC_CATCH_BEGIN();

        // Cast to const char* to std::string to ensure null termination at correct length
        shader = Shader::CreateShader(std::string(reinterpret_cast<const char*>(data.data()), data.size()));

        BC_CATCH_END_RETURN(nullptr);
        return shader;
    }

    std::shared_ptr<Asset> AssetImporter::ImportComputeShader(ImportParameters& import_parameters, const std::vector<uint8_t>& data)
    {
        std::shared_ptr<Shader> shader;
        BC_CATCH_BEGIN();

        // Cast to const char* to std::string to ensure null termination at correct length
        shader = Shader::CreateComputeShader(std::string(reinterpret_cast<const char*>(data.data()), data.size()));

        BC_CATCH_END_RETURN(nullptr);
        return shader;
    }

#pragma endregion

#pragma region AnimatorStateMachine Import

    std::shared_ptr<Asset> AssetImporter::ImportAnimatorStateMachine(ImportParameters& import_parameters, const std::vector<uint8_t>& data)
    {
        std::shared_ptr<Animation::StateMachine> state_machine;
        BC_CATCH_BEGIN();

        // Cast to const char* to std::string to ensure null termination at correct length
        state_machine = Animation::StateMachine::CreateStateMachine(std::string(reinterpret_cast<const char*>(data.data()), data.size()));

        BC_CATCH_END_RETURN(nullptr);
        return state_machine;
    }

#pragma endregion

#pragma region Humanoid / HumanoidMask Import

    std::shared_ptr<Asset> AssetImporter::ImportHumanoid(ImportParameters& import_parameters, const std::vector<uint8_t>& data)
    {
        std::shared_ptr<Humanoid> humanoid;
        BC_CATCH_BEGIN();

        // Cast to const char* to std::string to ensure null termination at correct length
        humanoid = Humanoid::CreateHumanoid(std::string(reinterpret_cast<const char*>(data.data()), data.size()));

        BC_CATCH_END_RETURN(nullptr);
        return humanoid;
    }

    std::shared_ptr<Asset> AssetImporter::ImportHumanoidMask(ImportParameters& import_parameters, const std::vector<uint8_t>& data)
    {
        std::shared_ptr<HumanoidMask> humanoid_mask;
        BC_CATCH_BEGIN();

        // Cast to const char* to std::string to ensure null termination at correct length
        humanoid_mask = HumanoidMask::CreateHumanoidMask(std::string(reinterpret_cast<const char*>(data.data()), data.size()));

        BC_CATCH_END_RETURN(nullptr);
        return humanoid_mask;
    }

#pragma endregion

#pragma region PhysicsMaterial Import

    std::shared_ptr<Asset> AssetImporter::ImportPhysicsMaterial(ImportParameters& import_parameters, const std::vector<uint8_t>& data)
    {
        std::shared_ptr<PhysicsMaterial> physics_material;
        BC_CATCH_BEGIN();

        // Cast to const char* to std::string to ensure null termination at correct length
        physics_material = PhysicsMaterial::CreatePhysicsMaterial(std::string(reinterpret_cast<const char*>(data.data()), data.size()));

        BC_CATCH_END_RETURN(nullptr);
        return physics_material;
    }

#pragma endregion

}