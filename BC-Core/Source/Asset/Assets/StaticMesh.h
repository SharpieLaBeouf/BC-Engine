#pragma once

// Core Headers
#include "Asset/Asset.h"

#include "Project/Scene/Bounds/Bounds.h"

// C++ Standard Library Headers
#include <vector>
#include <memory>

// External Vendor Library Headers

namespace BC
{

    class SubMesh
    {

    };

    class StaticMesh : public Asset
    {

    public:

		virtual AssetType GetType() const override { return AssetType::Mesh; }

		StaticMesh() = default;
		~StaticMesh() = default;

		StaticMesh(const StaticMesh& other);
		StaticMesh(StaticMesh&& other) noexcept;
        
		StaticMesh& operator=(const StaticMesh& other);
		StaticMesh& operator=(StaticMesh&& other) noexcept;

        const std::vector<SubMesh>& GetSubMeshes() const { return m_SubMeshes; }

		const Bounds_AABB& GetMeshBounds() const { return m_MeshBounds; }

    private:

		std::vector<SubMesh> m_SubMeshes{};

		Bounds_AABB m_MeshBounds{};
		bool m_ModifiedAABB = false;

    };

}