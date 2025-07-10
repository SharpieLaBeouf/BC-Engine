#pragma once

#include "Util/Platform.h"

#include <vulkan/vulkan.h>

#include <vulkan/vulkan.h>
#if defined(BC_PLATFORM_WINDOWS)
#include <vma/vk_mem_alloc.h>
#elif defined(BC_PLATFORM_LINUX)
#include <vk_mem_alloc.h>
#endif

#include <cstdint>
#include <cstring>
#include <stdexcept>

namespace BC
{
	class VulkanBuffer
	{

	public:

		VulkanBuffer() = default;

		VulkanBuffer(VmaAllocator allocator,
		             VkDeviceSize size,
		             VkBufferUsageFlags usage,
		             VmaMemoryUsage memory_usage,
		             VmaAllocationCreateFlags alloc_flags = 0);

		~VulkanBuffer();

		// No copy
		VulkanBuffer(const VulkanBuffer&) = delete;
		VulkanBuffer& operator=(const VulkanBuffer&) = delete;

		// Move
		VulkanBuffer(VulkanBuffer&& other) noexcept;
		VulkanBuffer& operator=(VulkanBuffer&& other) noexcept;

		// Upload data (must be host-visible or staging buffer used externally)
		void Upload(const void* data, VkDeviceSize size, VkDeviceSize offset = 0);

		// Map/unmap
		void* Map();
		void Unmap();

		// Accessors
		VkBuffer GetBuffer() const { return m_Buffer; }
		VmaAllocation GetAllocation() const { return m_Allocation; }
		VkDeviceSize GetSize() const { return m_Size; }

		bool IsMapped() const { return m_MappedPtr != nullptr; }

	    VulkanBuffer Clone(VkCommandPool cmd_pool, VkQueue queue, VkBufferUsageFlags usage) const;

	private:

		void Cleanup();

	private:

		VmaAllocator m_Allocator = nullptr;
		VkBuffer m_Buffer = VK_NULL_HANDLE;
		VmaAllocation m_Allocation = nullptr;
		VkDeviceSize m_Size = 0;
		void* m_MappedPtr = nullptr;
	};
}