#include "BC_PCH.h"
#include "GeometryEnvironment.h"

#include "Asset/AssetManagerAPI.h"

namespace BC
{

    void GeometryEnvironment::AddStaticMesh(MeshRendererComponent &mesh_component)
    {
        if (!mesh_component.GetActive())
            return;

        auto& material_handles = mesh_component.GetMaterialHandles();
        auto static_mesh_asset = AssetManager::GetAsset<StaticMesh>(mesh_component.GetStaticMesh());

        if (!static_mesh_asset || material_handles.empty())
            return;

        int material_index = 0;
        for (int i = 0; i < static_mesh_asset->GetSubMeshes().size(); ++i)
        {
            if (material_index >= material_handles.size())
                break;

            const auto& sub_mesh = static_mesh_asset->GetSubMeshes()[i];
            auto material_asset = AssetManager::GetAsset<Material>(material_handles[i]);

            if (!material_asset)
            {
                // Will use last material if mis-matched materials to sub meshes
                if (i <= material_handles.size() - 1)
                    ++material_index;

                continue;
            }

            switch (material_asset->GetMaterialRenderType())
            {
                case MaterialRenderType_Opaque:
                {
                    OpaqueMaterialGeometryMap[material_handles[i]].push_back({});
                    Util::GeometryData& data = OpaqueMaterialGeometryMap[material_handles[i]].back();

                    data.static_mesh.static_mesh_asset_handle = mesh_component.GetStaticMesh();
                    data.static_mesh.sub_mesh_index = i;
                    data.static_mesh.geom_type = Util::GeometryType_Static;
                    data.static_mesh.casting_shadow = mesh_component.GetCastingShadows();

                    break;
                }
                case MaterialRenderType_Mixed:
                {
                    MixedMaterialGeometryMap[material_handles[i]].push_back({});
                    Util::GeometryData& data = MixedMaterialGeometryMap[material_handles[i]].back();

                    data.static_mesh.static_mesh_asset_handle = mesh_component.GetStaticMesh();
                    data.static_mesh.sub_mesh_index = i;
                    data.static_mesh.geom_type = Util::GeometryType_Static;
                    data.static_mesh.casting_shadow = mesh_component.GetCastingShadows();

                    break;
                }
                case MaterialRenderType_Transparent:
                {
                    TransparentMaterialGeometryMap[material_handles[i]].push_back({});
                    Util::GeometryData& data = TransparentMaterialGeometryMap[material_handles[i]].back();

                    data.static_mesh.static_mesh_asset_handle = mesh_component.GetStaticMesh();
                    data.static_mesh.sub_mesh_index = i;
                    data.static_mesh.geom_type = Util::GeometryType_Static;
                    data.static_mesh.casting_shadow = mesh_component.GetCastingShadows();

                    break;
                }
            }

            // Will use last material if mis-matched materials to sub meshes
            if (i <= material_handles.size() - 1)
                ++material_index;
        }
    }

    void GeometryEnvironment::AddSkinnedMesh(SkinnedMeshRendererComponent &mesh_component)
    {

    }
}