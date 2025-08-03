#include "BC_PCH.h"
#include "StaticMesh.h"

#include "Core/Application.h"

namespace BC
{

    SubMesh::SubMesh(const BufferLayout& buffer_layout, const std::vector<uint8_t>& vertex_data, const std::vector<uint32_t>& indice_data)
    {
        // Validate Indices
        size_t vertex_stride = buffer_layout.GetStride();
        size_t vertex_count = vertex_data.size() / vertex_stride;

        for (uint32_t index : indice_data)
        {
            if (index >= vertex_count)
            {
                BC_CORE_ERROR("SubMesh::SubMesh: Could Not Create Sub Mesh - Index Out of Bounds (Vertices Size = {}, Index Out of Bounds = {}).", vertex_count, index);
                return;
            }
        }

        m_VertexBufferLayout = buffer_layout;
        m_IndexCount = indice_data.size();

        VulkanBuffer vertex_staging(
            Application::GetVulkanCore()->GetAllocator(),
            vertex_data.size(),
            vk::BufferUsageFlagBits::eTransferSrc,
            VMA_MEMORY_USAGE_CPU_ONLY
        );

        vertex_staging.Upload(vertex_data.data(), vertex_data.size()); // Size is already bytes uint8_t, just do size of vector

        m_VertexBuffer = std::make_shared<VulkanBuffer>(std::move(vertex_staging.Clone(
            vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
            VMA_MEMORY_USAGE_GPU_ONLY
        )));

        VulkanBuffer index_staging(
            Application::GetVulkanCore()->GetAllocator(),
            indice_data.size() * sizeof(uint32_t),
            vk::BufferUsageFlagBits::eTransferSrc,
            VMA_MEMORY_USAGE_CPU_ONLY
        );

        index_staging.Upload(indice_data.data(), indice_data.size() * sizeof(uint32_t)); // Size needs to be mul by size of uint32_t (4 bytes)

        m_IndexBuffer = std::make_shared<VulkanBuffer>(std::move(index_staging.Clone(
            vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
            VMA_MEMORY_USAGE_GPU_ONLY
        )));
    }

}