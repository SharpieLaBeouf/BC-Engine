#include "BC_PCH.h"
#include "VulkanBuffer.h"

#include "Debug/Logging.h"
#include "Debug/Assert.h"

namespace BC
{
	VulkanBuffer::VulkanBuffer(VmaAllocator allocator,
	                           VkDeviceSize size,
	                           VkBufferUsageFlags usage,
	                           VmaMemoryUsage memory_usage,
	                           VmaAllocationCreateFlags alloc_flags)
	    : m_Allocator(allocator), m_Size(size)
	{
		assert(allocator && "VulkanBuffer requires a valid VmaAllocator");

		VkBufferCreateInfo buffer_info{};
		buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_info.size = size;
		buffer_info.usage = usage;
		buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo alloc_info{};
		alloc_info.usage = memory_usage;
		alloc_info.flags = alloc_flags;

		if (vmaCreateBuffer(allocator, &buffer_info, &alloc_info, &m_Buffer, &m_Allocation, nullptr) != VK_SUCCESS)
			throw std::runtime_error("Failed to create VulkanBuffer!");
	}

	VulkanBuffer::~VulkanBuffer()
	{
		Cleanup();
	}

	VulkanBuffer::VulkanBuffer(VulkanBuffer&& other) noexcept
	{
		*this = std::move(other);
	}

	VulkanBuffer& VulkanBuffer::operator=(VulkanBuffer&& other) noexcept
	{
		if (this != &other)
		{
			Cleanup();

			m_Allocator = other.m_Allocator;
			m_Buffer = other.m_Buffer;
			m_Allocation = other.m_Allocation;
			m_Size = other.m_Size;
			m_MappedPtr = other.m_MappedPtr;

			other.m_Buffer = VK_NULL_HANDLE;
			other.m_Allocation = nullptr;
			other.m_Size = 0;
			other.m_MappedPtr = nullptr;
		}
		return *this;
	}

	void VulkanBuffer::Upload(const void* data, VkDeviceSize size, VkDeviceSize offset)
	{
		assert(data && "Upload data is null!");
		assert((offset + size) <= m_Size && "Upload size exceeds buffer bounds");

		void* dst = Map();
		std::memcpy(static_cast<char*>(dst) + offset, data, static_cast<size_t>(size));
		Unmap(); // Optional: you could keep it mapped for persistent buffers
	}

	void* VulkanBuffer::Map()
	{
		if (!m_MappedPtr)
		{
			vmaMapMemory(m_Allocator, m_Allocation, &m_MappedPtr);
		}
		return m_MappedPtr;
	}

	void VulkanBuffer::Unmap()
	{
		if (m_MappedPtr)
		{
			vmaUnmapMemory(m_Allocator, m_Allocation);
			m_MappedPtr = nullptr;
		}
	}

	void VulkanBuffer::Cleanup()
	{
		if (m_Buffer != VK_NULL_HANDLE && m_Allocation)
		{
			vmaDestroyBuffer(m_Allocator, m_Buffer, m_Allocation);
			m_Buffer = VK_NULL_HANDLE;
			m_Allocation = nullptr;
			m_Size = 0;
			m_MappedPtr = nullptr;
		}
	}

    VulkanBuffer VulkanBuffer::Clone(VkCommandPool cmd_pool, VkQueue queue, VkBufferUsageFlags usage) const
    {
        VulkanBuffer clone
        (
            m_Allocator,
            m_Size,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage,
            VMA_MEMORY_USAGE_GPU_ONLY
        );

        // Allocate and record copy command
        VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandPool = cmd_pool;
        alloc_info.commandBufferCount = 1;

        VkCommandBuffer cmd_buf;
        vkAllocateCommandBuffers(Application::GetVulkanCore()->GetLogicalDevice(), &alloc_info, &cmd_buf);

        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(cmd_buf, &begin_info);

        VkBufferCopy copy_region{};
        copy_region.srcOffset = 0;
        copy_region.dstOffset = 0;
        copy_region.size = m_Size;
        vkCmdCopyBuffer(cmd_buf, m_Buffer, clone.m_Buffer, 1, &copy_region);

        vkEndCommandBuffer(cmd_buf);

        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &cmd_buf;

        vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
        vkQueueWaitIdle(queue);
        vkFreeCommandBuffers(Application::GetVulkanCore()->GetLogicalDevice(), cmd_pool, 1, &cmd_buf);

        return clone;
    }
}
