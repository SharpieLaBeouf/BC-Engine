#include "BC_PCH.h"
#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Graphics/Vulkan/VulkanUtil.h"

namespace BC
{
    Texture2D::~Texture2D()
    {
        auto device = Application::GetVulkanCore()->GetLogicalDevice();

        if (m_ImageDescriptor != VK_NULL_HANDLE)
        {
            // Descriptor sets allocated from a pool with VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT
            vkFreeDescriptorSets(device, Application::GetVulkanCore()->GetStaticDescriptorPool(), 1, &m_ImageDescriptor);
            m_ImageDescriptor = VK_NULL_HANDLE;
        }

        if (m_Sampler != VK_NULL_HANDLE)
        {
            vkDestroySampler(device, m_Sampler, nullptr);
            m_Sampler = VK_NULL_HANDLE;
        }

        if (m_ImageView != VK_NULL_HANDLE)
        {
            vkDestroyImageView(device, m_ImageView, nullptr);
            m_ImageView = VK_NULL_HANDLE;
        }

        if (m_Image != VK_NULL_HANDLE)
        {
            vkDestroyImage(device, m_Image, nullptr);
            m_Image = VK_NULL_HANDLE;
        }

        if (m_ImageMemory != VK_NULL_HANDLE)
        {
            vkFreeMemory(device, m_ImageMemory, nullptr);
            m_ImageMemory = VK_NULL_HANDLE;
        }
    }

    uint32_t Texture2D::GetChannelsFromFormat(VkFormat format)
    {
        switch (format)
        {
            case VK_FORMAT_R8_UNORM:
            case VK_FORMAT_R8_SRGB:
            case VK_FORMAT_R8_UINT:
            case VK_FORMAT_R8_SINT:
                return 1;

            case VK_FORMAT_R8G8_UNORM:
            case VK_FORMAT_R8G8_SRGB:
            case VK_FORMAT_R8G8_UINT:
            case VK_FORMAT_R8G8_SINT:
                return 2;

            case VK_FORMAT_R8G8B8_UNORM:
            case VK_FORMAT_R8G8B8_SRGB:
            case VK_FORMAT_R8G8B8_UINT:
            case VK_FORMAT_R8G8B8_SINT:
            case VK_FORMAT_B8G8R8_UNORM:
            case VK_FORMAT_B8G8R8_SRGB:
            case VK_FORMAT_B8G8R8_UINT:
            case VK_FORMAT_B8G8R8_SINT:
                return 3;

            case VK_FORMAT_R8G8B8A8_UNORM:
            case VK_FORMAT_R8G8B8A8_SRGB:
            case VK_FORMAT_R8G8B8A8_UINT:
            case VK_FORMAT_R8G8B8A8_SINT:
            case VK_FORMAT_B8G8R8A8_UNORM:
            case VK_FORMAT_B8G8R8A8_SRGB:
            case VK_FORMAT_B8G8R8A8_UINT:
            case VK_FORMAT_B8G8R8A8_SINT:
                return 4;

            case VK_FORMAT_D32_SFLOAT:
            case VK_FORMAT_D16_UNORM:
            case VK_FORMAT_D24_UNORM_S8_UINT:
            case VK_FORMAT_D32_SFLOAT_S8_UINT:
                return 1; // Depth formats, treated as 1 channel

            default:
                return 0;
        }
        return 0;
    }

    uint32_t Texture2D::GetBytesPerChannel(VkFormat format)
    {
        switch (format)
        {
            case VK_FORMAT_R8_UNORM:
            case VK_FORMAT_R8G8B8A8_UNORM:
            case VK_FORMAT_R8G8_UNORM:
            case VK_FORMAT_R8G8B8_UNORM:
                return 1;

            case VK_FORMAT_R16_SFLOAT:
            case VK_FORMAT_R16G16B16A16_SFLOAT:
                return 2;

            case VK_FORMAT_R32_SFLOAT:
            case VK_FORMAT_R32G32B32A32_SFLOAT:
                return 4;
        }
        return 1;
    }

    std::shared_ptr<Texture2D> Texture2D::CreateTexture(const std::filesystem::path& texture_path, bool cache_data_cpu)
    {
        if (!std::filesystem::exists(texture_path))
            return nullptr;

		int width, height, channels;
		stbi_set_flip_vertically_on_load(true);
		unsigned char* data = stbi_load(texture_path.string().c_str(), &width, &height, &channels, 0);

        if (width == 0 || height == 0 || channels == 0)
        {
            stbi_image_free(data);
            return nullptr;
        }

        Texture2DSpecification specification = {};
        specification.width = width;
        specification.height = height;

        switch (channels)
        {
            case 1:     specification.format    = VK_FORMAT_R8_UNORM;           break;
            case 2:     specification.format    =  VK_FORMAT_R8G8_UNORM;        break;
            case 3:     specification.format    = VK_FORMAT_R8G8B8_UNORM;       break;
            case 4:     specification.format    = VK_FORMAT_R8G8B8A8_UNORM;     break;
            default:    specification.format    = VK_FORMAT_UNDEFINED;          break;
        }

        if (specification.format == VK_FORMAT_UNDEFINED)
        {
            stbi_image_free(data);
            return nullptr;
        }

        specification.generate_mips = true;
        specification.mip_levels = std::max(width, height);
        specification.mip_levels = specification.mip_levels > 1
            ? static_cast<uint32_t>(std::floor(std::log2(specification.mip_levels))) + 1
            : 1;
        specification.mip_levels = std::min(specification.mip_levels, 5u);

        std::shared_ptr<Texture2D> texture = std::make_shared<Texture2D>();
        texture->m_Specification = specification;

        if (cache_data_cpu)
        {
            uint32_t channels = GetChannelsFromFormat(specification.format);
            uint32_t bpc = GetBytesPerChannel(specification.format);
            auto data_size = specification.width * specification.height * channels * bpc;
            texture->m_CachedData.resize(data_size);
            texture->m_CachedDataFormat = specification.format;
            memcpy(texture->m_CachedData.data(), data, data_size);
        }

        BC_CATCH_BEGIN();

        texture->CreateTexture(data, specification.format);

        BC_CATCH_END_RETURN(nullptr);

        stbi_image_free(data);

        return texture;
    }

    std::shared_ptr<Texture2D> Texture2D::CreateTexture(const Texture2DSpecification& specification, const unsigned char* texture_data_in, VkFormat texture_data_in_format, bool cache_data_cpu)
    {
        if (!texture_data_in || specification.width == 0 || specification.height == 0 || GetChannelsFromFormat(texture_data_in_format) == 0)
            return nullptr;

        auto texture = std::make_shared<Texture2D>();
        texture->m_Specification = specification;

        if (cache_data_cpu)
        {
            uint32_t channels = GetChannelsFromFormat(texture_data_in_format);
            uint32_t bpc = GetBytesPerChannel(texture_data_in_format);
            auto data_size = specification.width * specification.height * channels * bpc;
            texture->m_CachedData.resize(data_size);
            texture->m_CachedDataFormat = texture_data_in_format;
            memcpy(texture->m_CachedData.data(), texture_data_in, data_size);
        }

        BC_CATCH_BEGIN();

        texture->CreateTexture(texture_data_in, texture_data_in_format);

        BC_CATCH_END_RETURN(nullptr);

        return texture;
    }

    void Texture2D::CreateTexture(const unsigned char* texture_data_in, VkFormat texture_data_in_format)
    {
        const auto& spec = m_Specification;
        auto vulkan_core = Application::GetVulkanCore();

        // --- Create Image ---
        VkImageCreateInfo image_info{};
        image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_info.imageType = VK_IMAGE_TYPE_2D;
        image_info.format = spec.format;
        image_info.extent.width = spec.width;
        image_info.extent.height = spec.height;
        image_info.extent.depth = 1;
        image_info.mipLevels = spec.generate_mips ? spec.mip_levels : 1;
        image_info.arrayLayers = 1;
        image_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                        VK_IMAGE_USAGE_SAMPLED_BIT;

        if (texture_data_in)
            image_info.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

        if (spec.is_render_target)
            image_info.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        if (spec.is_storage)
            image_info.usage |= VK_IMAGE_USAGE_STORAGE_BIT;

        image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        BC_THROW(vkCreateImage(vulkan_core->GetLogicalDevice(), &image_info, nullptr, &m_Image) == VK_SUCCESS, "Texture2D::CreateTexture: Could Not Create Image.");

        VkMemoryRequirements mem_requirements;
        vkGetImageMemoryRequirements(vulkan_core->GetLogicalDevice(), m_Image, &mem_requirements);

        VkMemoryAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = mem_requirements.size;
        alloc_info.memoryTypeIndex = vulkan_core->FindMemoryType(mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        BC_THROW(vkAllocateMemory(vulkan_core->GetLogicalDevice(), &alloc_info, nullptr, &m_ImageMemory) == VK_SUCCESS, "Texture2D::CreateTexture: Could Not Allocate Image Memory.");
        BC_THROW(vkBindImageMemory(vulkan_core->GetLogicalDevice(), m_Image, m_ImageMemory, 0) == VK_SUCCESS, "Texture2D::CreateTexture: Could Not Bind Image Memory.");

        // --- Upload texture ---
        if (texture_data_in)
        {
            VkDeviceSize image_size = spec.width * spec.height *
                                    GetChannelsFromFormat(texture_data_in_format) *
                                    GetBytesPerChannel(texture_data_in_format);

            // Create staging buffer and upload
            VkBuffer staging_buffer;
            VkDeviceMemory staging_memory;
            Util::CreateBuffer(vulkan_core->GetLogicalDevice(), vulkan_core->GetPhysicalDevice(), image_size,
                                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                staging_buffer, staging_memory);

            void* data = nullptr;
            vkMapMemory(vulkan_core->GetLogicalDevice(), staging_memory, 0, image_size, 0, &data);
            memcpy(data, texture_data_in, static_cast<size_t>(image_size));
            vkUnmapMemory(vulkan_core->GetLogicalDevice(), staging_memory);

            // Submit layout transitions and copy
            auto command_pool = vulkan_core->GetThreadUploadCommandPool();
            auto cmd_buf = vulkan_core->BeginSingleUseCommandBuffer(command_pool);

            Util::TransitionImageLayout(cmd_buf, m_Image, spec.format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, spec.mip_levels);

            Util::CopyBufferToImage(cmd_buf, staging_buffer, m_Image, spec.width, spec.height);

            if (spec.generate_mips)
            {
                Util::GenerateMipmaps(cmd_buf, m_Image, spec.format, spec.width, spec.height, spec.mip_levels);
            }
            else
            {
                Util::TransitionImageLayout(cmd_buf, m_Image, spec.format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1);
            }

            vulkan_core->EndSingleUseCommandBuffer(vulkan_core->GetGraphicsQueue(), command_pool, cmd_buf);

            vkDestroyBuffer(vulkan_core->GetLogicalDevice(), staging_buffer, nullptr);
            vkFreeMemory(vulkan_core->GetLogicalDevice(), staging_memory, nullptr);
        }

        // --- Create Image View ---
        VkImageViewCreateInfo view_info{};
        view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image = m_Image;
        view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format = spec.format;
        view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        view_info.subresourceRange.baseMipLevel = 0;
        view_info.subresourceRange.levelCount = spec.mip_levels;
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount = 1;

        BC_THROW(vkCreateImageView(vulkan_core->GetLogicalDevice(), &view_info, nullptr, &m_ImageView) == VK_SUCCESS, "Texture2D::CreateTexture: Could Not Create Image View.");

        // --- Create Sampler ---
        VkSamplerCreateInfo sampler_info{};
        sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler_info.magFilter = spec.mag_filter;
        sampler_info.minFilter = spec.min_filter;
        sampler_info.addressModeU = spec.address_mode_u;
        sampler_info.addressModeV = spec.address_mode_v;
        sampler_info.addressModeW = spec.address_mode_v;
        sampler_info.anisotropyEnable = VK_TRUE;
        sampler_info.maxAnisotropy = 16.0f;
        sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        sampler_info.unnormalizedCoordinates = VK_FALSE;
        sampler_info.compareEnable = VK_FALSE;
        sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        sampler_info.minLod = 0.0f;
        sampler_info.maxLod = static_cast<float>(spec.mip_levels);
        sampler_info.mipLodBias = 0.0f;

        BC_THROW(vkCreateSampler(vulkan_core->GetLogicalDevice(), &sampler_info, nullptr, &m_Sampler) == VK_SUCCESS, "Texture2D::CreateTexture: Could Not Create Sampler.");
    }

    VkDescriptorSet Texture2D::GetDescriptor()
    {
        if (m_ImageDescriptor != VK_NULL_HANDLE)
            return m_ImageDescriptor;

        auto layout = Application::GetVulkanCore()->GetImageSamplerSetLayout();
        VkDescriptorSetAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorPool = Application::GetVulkanCore()->GetStaticDescriptorPool();
        alloc_info.descriptorSetCount = 1;
        alloc_info.pSetLayouts = &layout;

        BC_THROW(vkAllocateDescriptorSets(Application::GetVulkanCore()->GetLogicalDevice(), &alloc_info, &m_ImageDescriptor) == VK_SUCCESS, "Texture2D::CreateTexture: Could Not Allocate Descriptor Set.");

        VkDescriptorImageInfo image_info{};
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_info.imageView = m_ImageView;
        image_info.sampler = m_Sampler;

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = m_ImageDescriptor;
        write.dstBinding = 0;
        write.dstArrayElement = 0;
        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write.descriptorCount = 1;
        write.pImageInfo = &image_info;

        vkUpdateDescriptorSets(Application::GetVulkanCore()->GetLogicalDevice(), 1, &write, 0, nullptr);

        return m_ImageDescriptor;
    }
}