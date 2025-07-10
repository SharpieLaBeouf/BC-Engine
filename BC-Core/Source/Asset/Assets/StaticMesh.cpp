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

        // Allocate & Upload Vertex Buffer
        m_VertexBuffer = std::make_shared<VulkanBuffer>(
            Application::GetVulkanCore()->GetAllocator(),
            vertex_data.size(),
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VMA_MEMORY_USAGE_GPU_ONLY
        );

        // Allocate & Upload Index Buffer
        m_IndexBuffer = std::make_shared<VulkanBuffer>(
            Application::GetVulkanCore()->GetAllocator(),
            indice_data.size() * sizeof(uint32_t),
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VMA_MEMORY_USAGE_GPU_ONLY
        );

        // Upload using staging externally
        //VulkanCore::UploadToGPUBuffer(m_VertexBuffer, vertex_data.data(), vertex_data.size());
        //VulkanCore::UploadToGPUBuffer(m_IndexBuffer, indice_data.data(), indice_data.size() * sizeof(uint32_t));
    }

}