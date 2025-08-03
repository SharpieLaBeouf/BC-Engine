#pragma once

// Core Headers
#include "Asset/Asset.h"

#include "Project/Scene/Bounds/Bounds.h"

#include "Graphics/Util/BufferLayout.h"
#include "Graphics/Vulkan/VulkanBuffer.h"

// C++ Standard Library Headers
#include <vector>
#include <memory>
#include <mutex>

// External Vendor Library Headers
#include <vulkan/vulkan_raii.hpp>

namespace BC
{
	namespace Util
	{
		static inline BufferLayout s_DefaultBufferLayout = 
		{
			{ ShaderDataType_Float3,    "a_Position"       },
			{ ShaderDataType_Float3,    "a_Normal"         },
			{ ShaderDataType_Float2,    "a_TexCoord"       },
			{ ShaderDataType_Float3,    "a_Tangent"        },
			{ ShaderDataType_Float3,    "a_Bitangent"      },
			{ ShaderDataType_Int4,      "a_BoneID"         },
			{ ShaderDataType_Float4,    "a_BoneWeight"     }
		};
	}

    class SubMesh
    {

	public:

		SubMesh() = default;
		SubMesh(const BufferLayout& buffer_layout, const std::vector<uint8_t>& vertex_data, const std::vector<uint32_t>& indice_data);
		~SubMesh() = default;
		
		SubMesh(const SubMesh& other) = default;
		SubMesh(SubMesh&& other) noexcept = default;
		SubMesh& operator=(const SubMesh& other) = default;
		SubMesh& operator=(SubMesh&& other) noexcept = default;

		const BufferLayout& GetVertexBufferLayout() const { return m_VertexBufferLayout; }
		uint32_t GetIndexCount() const { return m_IndexCount; }

		const std::shared_ptr<VulkanBuffer>& GetVertexBuffer() const { return m_VertexBuffer; }
		const std::shared_ptr<VulkanBuffer>& GetIndexBuffer() const { return m_IndexBuffer; }

	public:

		static SubMesh DuplicateSubMesh(const SubMesh& source)
		{
			
		}

	private:

		BufferLayout m_VertexBufferLayout = {};

		std::shared_ptr<VulkanBuffer> m_VertexBuffer;
		std::shared_ptr<VulkanBuffer> m_IndexBuffer;
		
		uint32_t m_IndexCount = 0;
    };

    class StaticMesh : public Asset
    {

    public:

		virtual AssetType GetType() const override { return AssetType::Mesh; }

		StaticMesh() = default;
		StaticMesh(const std::vector<SubMesh>& sub_meshes, const Bounds_AABB& mesh_bounds) : m_SubMeshes(sub_meshes), m_MeshBounds(mesh_bounds) { }
		~StaticMesh() = default;

		StaticMesh(const StaticMesh& other) = default;
		StaticMesh(StaticMesh&& other) noexcept = default;

		StaticMesh& operator=(const StaticMesh& other) = default;
		StaticMesh& operator=(StaticMesh&& other) noexcept = default;

		const std::vector<SubMesh>& GetSubMeshes() const { return m_SubMeshes; }
		const Bounds_AABB& GetMeshBounds() const { return m_MeshBounds; }

	public:

		static std::shared_ptr<StaticMesh> DuplicateStaticMesh(const std::shared_ptr<StaticMesh>& source)
		{
			std::shared_ptr<StaticMesh> dest_mesh = std::make_shared<StaticMesh>();
			dest_mesh->m_MeshBounds = source->m_MeshBounds;
			dest_mesh->m_ModifiedAABB = source->m_ModifiedAABB;

			for (const auto& sub_mesh : source->m_SubMeshes)
			{
				dest_mesh->m_SubMeshes.push_back(SubMesh::DuplicateSubMesh(sub_mesh));
			}
		}

    private:

		std::vector<SubMesh> m_SubMeshes = {};

		Bounds_AABB m_MeshBounds{};
		bool m_ModifiedAABB = false;

    };

}