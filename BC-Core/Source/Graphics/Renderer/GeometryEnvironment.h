#pragma once

// Core Headers
#include "Asset/Asset.h"

#include "Asset/Assets/StaticMesh.h"
#include "Asset/Assets/Material.h"

#include "Project/Scene/Components/MeshComponents.h"

// C++ Standard Library Headers
#include <unordered_map>

// External Vendor Library Headers

namespace BC
{
    namespace Util
    {
        using GeometryType = uint8_t;
        enum : GeometryType
        {
            GeometryType_Static,
            GeometryType_Skinned
        };

        struct StaticSubMeshData
        {
            AssetHandle static_mesh_asset_handle;
            uint32_t sub_mesh_index;
            GeometryType geom_type;
            bool casting_shadow;
            glm::mat4 transform_matrix;
            GUID entity_guid;
            
            // TODO: Add other requirements
        };
        
        struct SkinnedSubMeshData
        {
            AssetHandle static_mesh_asset_handle;
            uint32_t sub_mesh_index;
            GeometryType geom_type;
            bool casting_shadow;
            glm::mat4 transform_matrix;
            GUID entity_guid;

            size_t bone_buffer_offset;  // Offset in bone buffer SSBO where this bones final transformations are stored

            // TODO: Add other requirements
        };

        struct GeometryData
        {
            union
            {
                StaticSubMeshData static_mesh;
                SkinnedSubMeshData skinned_mesh;
            };
        };
    }

    struct GeometryEnvironment
    {
        // Key = Opaque Material Handle, Value = Vector of Geometry Data
        std::unordered_map<AssetHandle, std::vector<Util::GeometryData>> OpaqueMaterialGeometryMap;
        
        // Key = Mixed Transparent Material Handle, Value = Vector of Geometry Data
        std::unordered_map<AssetHandle, std::vector<Util::GeometryData>> MixedMaterialGeometryMap;
        
        // Key = Transparent Material Handle, Value = Vector of Geometry Data
        std::unordered_map<AssetHandle, std::vector<Util::GeometryData>> TransparentMaterialGeometryMap;

        void AddStaticMesh(MeshRendererComponent& mesh_component);

        void AddSkinnedMesh(SkinnedMeshRendererComponent& mesh_component);
    };

}