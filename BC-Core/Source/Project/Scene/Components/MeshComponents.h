#pragma once

// Core Headers
#include "Component Base.h"

#include "Asset/Asset.h"

// C++ Standard Library Headers

// External Vendor Library Headers

namespace YAML 
{
	class Emitter;
	class Node;
}

namespace BC
{

    struct LODMeshComponent : public ComponentBase
    {
    
    public:

        LODMeshComponent() = default;
        ~LODMeshComponent() = default;

        LODMeshComponent(const LODMeshComponent& other);
        LODMeshComponent(LODMeshComponent&& other) noexcept;
        LODMeshComponent& operator=(const LODMeshComponent& other);
        LODMeshComponent& operator=(LODMeshComponent&& other) noexcept;

        ComponentType GetType() override { return ComponentType::LODMeshComponent; }

        void SceneSerialise(YAML::Emitter& out) const override;
        bool SceneDeserialise(const YAML::Node& data) override;

    private:
        

    };
    
    struct MeshRendererComponent : public ComponentBase
    {
    
    public:

        MeshRendererComponent() = default;
        ~MeshRendererComponent() = default;

        MeshRendererComponent(const MeshRendererComponent& other);
        MeshRendererComponent(MeshRendererComponent&& other) noexcept;
        MeshRendererComponent& operator=(const MeshRendererComponent& other);
        MeshRendererComponent& operator=(MeshRendererComponent&& other) noexcept;

        ComponentType GetType() override { return ComponentType::MeshRendererComponent; }

        void SceneSerialise(YAML::Emitter& out) const override;
        bool SceneDeserialise(const YAML::Node& data) override;

        void SetActive(bool active) { m_Active = active; }
        void SetStaticMesh(AssetHandle mesh_handle) { m_StaticMeshHandle = mesh_handle; }
        void SetCastingShadows(bool cast_shadows) { m_CastingShadow = cast_shadows; }

        bool GetActive() const { return m_Active; }
        AssetHandle GetStaticMesh() const { return m_StaticMeshHandle; }
        bool GetCastingShadows() const { return m_CastingShadow; }
        std::vector<AssetHandle>& GetMaterialHandles() { return m_MaterialHandles; }

        void UpdateOctree()         { m_OctreeNeedsUpdate = true; }
        void UpdateBoundingBox()    { m_BoundingBoxNeedsUpdate = true; }

    private:

        AssetHandle m_StaticMeshHandle = NULL_GUID;
        std::vector<AssetHandle> m_MaterialHandles = {};

        bool m_Active = false;
        bool m_CastingShadow = false;
        
        bool m_OctreeNeedsUpdate = true;
        bool m_BoundingBoxNeedsUpdate = true;

    };
    
    struct SkinnedMeshRendererComponent : public ComponentBase
    {
    
    public:

        SkinnedMeshRendererComponent() = default;
        ~SkinnedMeshRendererComponent() = default;

        SkinnedMeshRendererComponent(const SkinnedMeshRendererComponent& other);
        SkinnedMeshRendererComponent(SkinnedMeshRendererComponent&& other) noexcept;
        SkinnedMeshRendererComponent& operator=(const SkinnedMeshRendererComponent& other);
        SkinnedMeshRendererComponent& operator=(SkinnedMeshRendererComponent&& other) noexcept;

        ComponentType GetType() override { return ComponentType::SkinnedMeshRendererComponent; }

        void SceneSerialise(YAML::Emitter& out) const override;
        bool SceneDeserialise(const YAML::Node& data) override;

        void SetActive(bool active) { m_Active = active; }
        void SetSkinnedMesh(AssetHandle mesh_handle) { m_SkinnedMeshHandle = mesh_handle; }
        void SetCastingShadows(bool cast_shadows) { m_CastingShadow = cast_shadows; }

        bool GetActive() const { return m_Active; }
        AssetHandle GetSkinnedMesh() const { return m_SkinnedMeshHandle; }
        bool GetCastingShadows() const { return m_CastingShadow; }
        std::vector<AssetHandle>& GetMaterialHandles() { return m_MaterialHandles; }

        void UpdateOctree()         { m_OctreeNeedsUpdate = true; }
        void UpdateBoundingBox()    { m_BoundingBoxNeedsUpdate = true; }

    private:

        AssetHandle m_SkinnedMeshHandle = NULL_GUID;
        std::vector<AssetHandle> m_MaterialHandles = {};

        bool m_Active = false;
        bool m_CastingShadow = false;
        
        bool m_OctreeNeedsUpdate = true;
        bool m_BoundingBoxNeedsUpdate = true;
        
    };

}