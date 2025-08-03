#pragma once

#include "Util/Platform.h"

#include <vulkan/vulkan_raii.hpp>
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
		             vk::DeviceSize size,
		             vk::BufferUsageFlags usage,
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
		void Upload(const void* data, vk::DeviceSize size, vk::DeviceSize offset = 0);

		// Map/unmap
		bool IsHostVisible() const;
		void* Map();
		void Unmap();

		// Accessors
		const vk::Buffer& GetBuffer() const { return m_Buffer; }
		vk::Buffer& GetBuffer() { return m_Buffer; }

		VmaAllocation GetAllocation() const { return m_Allocation; }
		vk::DeviceSize GetSize() const { return m_Size; }

		bool IsMapped() const { return m_MappedPtr != nullptr; }

	    VulkanBuffer Clone(vk::BufferUsageFlags usage, VmaMemoryUsage memory_usage) const;

	private:

		void Cleanup();

	private:

		VmaAllocator m_Allocator = nullptr;
		VmaAllocation m_Allocation = nullptr;

		vk::Buffer m_Buffer = VK_NULL_HANDLE;
		vk::DeviceSize m_Size = 0;
		
		void* m_MappedPtr = nullptr;
	};
}