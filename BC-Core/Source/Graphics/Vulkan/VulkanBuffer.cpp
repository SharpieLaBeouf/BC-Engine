#include "BC_PCH.h"
#include "VulkanBuffer.h"

#include "Debug/Logging.h"
#include "Debug/Assert.h"

namespace BC
{
	VulkanBuffer::VulkanBuffer(VmaAllocator allocator,
	                           vk::DeviceSize size,
	                           vk::BufferUsageFlags usage,
	                           VmaMemoryUsage memory_usage,
	                           VmaAllocationCreateFlags alloc_flags)
	    : m_Allocator(allocator), m_Size(size)
	{
		BC_THROW(allocator, "VulkanBuffer::VulkanBuffer: Requires a Valid VmaAllocator.");

		vk::BufferCreateInfo buffer_info{};
		buffer_info.setSize(size);
		buffer_info.setUsage(usage);
		buffer_info.setSharingMode(vk::SharingMode::eExclusive);

		VmaAllocationCreateInfo alloc_info{};
		alloc_info.usage = memory_usage;
		alloc_info.flags = alloc_flags;

		VkBuffer raw_buffer = VK_NULL_HANDLE;
		BC_THROW(vmaCreateBuffer(
			allocator,
			buffer_info,
			&alloc_info,
			&raw_buffer,
			&m_Allocation,
			nullptr
		) == VK_SUCCESS, "VulkanBuffer::VulkanBuffer: Failed to Create Vulkan Buffer.");

		m_Buffer = raw_buffer;
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

	void VulkanBuffer::Upload(const void* data, vk::DeviceSize size, vk::DeviceSize offset)
	{
		BC_THROW(data, "VulkanBuffer::Upload: Upload Data is NULL.");
		BC_THROW((offset + size) <= m_Size, "VulkanBuffer::Upload: Upload Size Exceeds Buffer Bounds.");

		void* dst = Map();
		if (!dst)
		{
			BC_CORE_WARN("VulkanBuffer::Upload: Buffer is not host-visible.");
			return;
		}

		std::memcpy(static_cast<char*>(dst) + offset, data, static_cast<size_t>(size));
		Unmap();
	}

	bool VulkanBuffer::IsHostVisible() const
	{
		VmaAllocationInfo alloc_info;
		vmaGetAllocationInfo(m_Allocator, m_Allocation, &alloc_info);
		return (alloc_info.memoryType & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0;
	}

	void* VulkanBuffer::Map()
	{
		if (!IsHostVisible())
		{
			BC_CORE_WARN("VulkanBuffer::Map: Buffer is not host-visible.");
			return nullptr;
		}

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
		if (m_Buffer && m_Allocation)
		{
			vmaDestroyBuffer(m_Allocator, m_Buffer, m_Allocation);
			m_Buffer = VK_NULL_HANDLE;
			m_Allocation = nullptr;
			m_Size = 0;
			m_MappedPtr = nullptr;
		}
	}

    VulkanBuffer VulkanBuffer::Clone(vk::BufferUsageFlags usage, VmaMemoryUsage memory_usage) const
    {
        VulkanBuffer clone
        (
            m_Allocator,
            m_Size,
            vk::BufferUsageFlagBits::eTransferDst | usage,
            memory_usage
        );

        // Allocate and record copy command
		auto vulkan_core = Application::GetVulkanCore();
		auto& cmd_pool = vulkan_core->GetThreadCommandPool();
		auto cmd_buffer = vulkan_core->BeginSingleUseCommandBuffer(cmd_pool);

        vk::BufferCopy copy_region = {};
		copy_region.setSrcOffset(0);
		copy_region.setDstOffset(0);
		copy_region.setSize((m_Size + 3) & ~vk::DeviceSize(3)); // Align to 4 bytes

		cmd_buffer.copyBuffer(m_Buffer, clone.m_Buffer, copy_region);

		vulkan_core->EndSingleUseCommandBuffer(vulkan_core->GetGraphicsQueue(), cmd_pool, cmd_buffer);

        return clone;
    }
}
