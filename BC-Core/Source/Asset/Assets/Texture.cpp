#include "BC_PCH.h"
#include "Texture.h"

#define STBI_NO_SIMD
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Graphics/Vulkan/VulkanUtil.h"

#include "Util/FileUtil.h"

namespace BC
{
    
    Texture2D::~Texture2D()
    {
        auto vma_allocator = Application::GetVulkanCore()->GetAllocator();

        m_ImageView.reset();
        m_Sampler.reset();
        m_ImageDescriptorSet.reset();

        // Destroy image and allocation with VMA
        if (m_Image && m_ImageAllocation)
        {
            vmaDestroyImage(vma_allocator, m_Image, m_ImageAllocation);
            m_Image = VK_NULL_HANDLE;
            m_ImageAllocation = nullptr;
        }
    }

    uint32_t Texture2D::GetChannelsFromFormat(vk::Format format)
    {
        switch (format)
        {
            case vk::Format::eR8Unorm:
            case vk::Format::eR8Srgb:
            case vk::Format::eR8Uint:
            case vk::Format::eR8Sint:
                return 1;

            case vk::Format::eR8G8Unorm:
            case vk::Format::eR8G8Srgb:
            case vk::Format::eR8G8Uint:
            case vk::Format::eR8G8Sint:
                return 2;

            case vk::Format::eR8G8B8Unorm:
            case vk::Format::eR8G8B8Srgb:
            case vk::Format::eR8G8B8Uint:
            case vk::Format::eR8G8B8Sint:
            case vk::Format::eB8G8R8Unorm:
            case vk::Format::eB8G8R8Srgb:
            case vk::Format::eB8G8R8Uint:
            case vk::Format::eB8G8R8Sint:
                return 3;

            case vk::Format::eR8G8B8A8Unorm:
            case vk::Format::eR8G8B8A8Srgb:
            case vk::Format::eR8G8B8A8Uint:
            case vk::Format::eR8G8B8A8Sint:
            case vk::Format::eB8G8R8A8Unorm:
            case vk::Format::eB8G8R8A8Srgb:
            case vk::Format::eB8G8R8A8Uint:
            case vk::Format::eB8G8R8A8Sint:
                return 4;

            case vk::Format::eD32Sfloat:
            case vk::Format::eD16Unorm:
            case vk::Format::eD24UnormS8Uint:
            case vk::Format::eD32SfloatS8Uint:
                return 1; // Depth formats, treated as 1 channel

            default:
                return 0;
        }
        return 0;
    }

    uint32_t Texture2D::GetBytesPerChannel(vk::Format format)
    {
        switch (format)
        {
            case vk::Format::eR8Unorm:
            case vk::Format::eR8G8B8A8Unorm:
            case vk::Format::eR8G8Unorm:
            case vk::Format::eR8G8B8Unorm:
                return 1;

            case vk::Format::eR16Sfloat:
            case vk::Format::eR16G16B16A16Sfloat:
                return 2;

            case vk::Format::eR32Sfloat:
            case vk::Format::eR32G32B32A32Sfloat:
                return 4;
        }
        return 1;
    }

    std::shared_ptr<Texture2D> Texture2D::CreateTexture(const std::filesystem::path& texture_path, bool cache_data_cpu)
    {
        if (!std::filesystem::exists(texture_path))
            return nullptr;

        std::shared_ptr<Texture2D> texture_asset = nullptr;
        BC_CATCH_BEGIN();

        texture_asset = Texture2D::CreateTextureFromFileBytes(Util::LoadDataStreamFromFile(texture_path), cache_data_cpu);

        BC_CATCH_END_RETURN(nullptr);

        return texture_asset;
    }

    std::shared_ptr<Texture2D> Texture2D::CreateTexture(const Texture2DSpecification& specification, const unsigned char* texture_data_in, vk::Format texture_data_in_format, bool cache_data_cpu)
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

    std::shared_ptr<Texture2D> Texture2D::CreateTextureFromFileBytes(const std::vector<uint8_t>& texture_file_bytes, bool cache_cpu_data)
    {
        unsigned char* image_data = nullptr;
        std::shared_ptr<Texture2D> texture = nullptr;

        BC_CATCH_BEGIN();

        int width, height, file_channels;
        if (!stbi_info_from_memory(texture_file_bytes.data(), static_cast<int>(texture_file_bytes.size()), &width, &height, &file_channels))
            return nullptr;

        if (width == 0 || height == 0 || file_channels == 0)
            return nullptr;

        int desired_channels = (file_channels == 3) ? 4 : 0;
        int actual_channels = 0;

        stbi_set_flip_vertically_on_load(true);
        image_data = stbi_load_from_memory(
            texture_file_bytes.data(),
            static_cast<int>(texture_file_bytes.size()),
            &width,
            &height,
            &actual_channels,
            desired_channels
        );

        if (!image_data || width == 0 || height == 0 || actual_channels == 0)
            return nullptr;

        Texture2DSpecification specification = {};
        specification.width = width;
        specification.height = height;

        switch (actual_channels)
        {
            case 1: specification.format = vk::Format::eR8Unorm; break;
            case 2: specification.format = vk::Format::eR8G8Unorm; break;
            case 3: specification.format = vk::Format::eR8G8B8Unorm; break; // Shouldn't technically hit as we force 4 if 3 channels
            case 4: specification.format = vk::Format::eR8G8B8A8Unorm; break;
            default: specification.format = vk::Format::eUndefined; break;
        }

        if (specification.format == vk::Format::eUndefined)
        {
            stbi_image_free(image_data);
            return nullptr;
        }

        specification.generate_mips = true;
        specification.mip_levels = std::max(width, height);
        specification.mip_levels = specification.mip_levels > 1
            ? static_cast<uint32_t>(std::floor(std::log2(specification.mip_levels))) + 1
            : 1;
        specification.mip_levels = std::min(specification.mip_levels, 5u);

        texture = Texture2D::CreateTexture(specification, image_data, specification.format, false);
        stbi_image_free(image_data);

        BC_CATCH_END_FUNC([&]() { texture.reset(); texture = nullptr; if (image_data) stbi_image_free(image_data); });

        return texture;
    }

    void Texture2D::CreateTexture(const unsigned char* texture_data_in, vk::Format texture_data_in_format)
    {
        const auto& spec = m_Specification;
        auto vulkan_core = Application::GetVulkanCore();
        auto vma_allocator = vulkan_core->GetAllocator();
        auto& device = vulkan_core->GetLogicalDevice();

        // --- Create Image with VMA ---
        vk::ImageCreateInfo image_info = {};
        image_info.setImageType(vk::ImageType::e2D);
        image_info.setFormat(static_cast<vk::Format>(spec.format));
        image_info.setExtent({ spec.width, spec.height, 1 });
        image_info.setMipLevels(spec.generate_mips ? spec.mip_levels : 1);
        image_info.setArrayLayers(1);
        image_info.setSamples(vk::SampleCountFlagBits::e1);
        image_info.setTiling(vk::ImageTiling::eOptimal);
        image_info.setUsage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled);
        
        if (texture_data_in)
            image_info.usage |= vk::ImageUsageFlagBits::eTransferSrc;
        if (spec.is_render_target)
            image_info.usage |= vk::ImageUsageFlagBits::eColorAttachment;
        if (spec.is_storage)
            image_info.usage |= vk::ImageUsageFlagBits::eStorage;
        
            image_info.setSharingMode(vk::SharingMode::eExclusive);
        image_info.setInitialLayout(vk::ImageLayout::eUndefined);

        VmaAllocationCreateInfo alloc_info = {};
        alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        
        VkImage raw_image = VK_NULL_HANDLE;

        BC_THROW(
            vmaCreateImage(
            vma_allocator,
            image_info,
            &alloc_info,
            &raw_image,
            &m_ImageAllocation,
            nullptr
            ) == VK_SUCCESS,
            "Texture2D::CreateTexture: Could Not Create Image."
        );

        m_Image = raw_image;

        // --- Upload texture ---
        if (texture_data_in)
        {
            vk::DeviceSize image_size = spec.width * spec.height *
                GetChannelsFromFormat(texture_data_in_format) *
                GetBytesPerChannel(texture_data_in_format);

            // Create staging buffer and upload
            VulkanBuffer staging_buffer(
                vma_allocator,
                image_size,
                vk::BufferUsageFlagBits::eTransferSrc,
                VMA_MEMORY_USAGE_CPU_ONLY
            );
            staging_buffer.Upload(texture_data_in, image_size);

            // Submit layout transitions and copy
            auto& command_pool = vulkan_core->GetThreadCommandPool();
            auto cmd_buf = vulkan_core->BeginSingleUseCommandBuffer(command_pool);

            Util::TransitionImageLayout(cmd_buf, m_Image, spec.format, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, spec.mip_levels);
            Util::CopyBufferToImage(cmd_buf, staging_buffer.GetBuffer(), m_Image, spec.width, spec.height);

            if (spec.generate_mips)
            {
                Util::GenerateMipmaps(cmd_buf, m_Image, spec.format, spec.width, spec.height, spec.mip_levels);
            }
            else
            {
                Util::TransitionImageLayout(cmd_buf, m_Image, spec.format, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, 1);
            }

            vulkan_core->EndSingleUseCommandBuffer(vulkan_core->GetGraphicsQueue(), command_pool, cmd_buf);
        }

        // --- Create Image View (RAII) ---
        vk::ImageViewCreateInfo view_info = {};
        view_info.setImage(m_Image);
        view_info.setViewType(vk::ImageViewType::e2D);
        view_info.setFormat(spec.format);
        view_info.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor);
        view_info.subresourceRange.setBaseMipLevel(0);
        view_info.subresourceRange.setLevelCount(spec.mip_levels);
        view_info.subresourceRange.setBaseArrayLayer(0);
        view_info.subresourceRange.setLayerCount(1);

        m_ImageView.emplace(device, view_info);

        // --- Create Sampler (RAII) ---
        vk::SamplerCreateInfo sampler_info = {};
        sampler_info.setMagFilter(spec.mag_filter);
        sampler_info.setMinFilter(spec.min_filter);
        sampler_info.setAddressModeU(spec.address_mode_u);
        sampler_info.setAddressModeV(spec.address_mode_v);
        sampler_info.setAddressModeW(spec.address_mode_v);
        sampler_info.setAnisotropyEnable(VK_TRUE);
        sampler_info.setMaxAnisotropy(16.0f);
        sampler_info.setBorderColor(vk::BorderColor::eIntOpaqueBlack);
        sampler_info.setUnnormalizedCoordinates(VK_FALSE);
        sampler_info.setCompareEnable(VK_FALSE);
        sampler_info.setMipmapMode(vk::SamplerMipmapMode::eLinear);
        sampler_info.setMinLod(0.0f);
        sampler_info.setMaxLod(static_cast<float>(spec.mip_levels));
        sampler_info.setMipLodBias(0.0f);

        m_Sampler.emplace(device, sampler_info);

        // --- Create Descriptor Set (RAII) ---
        auto& layout = Application::GetGUILayer()->GetImGuiDescriptorSetLayout();
        vk::DescriptorSetAllocateInfo alloc_info_ds = {};
        alloc_info_ds.setDescriptorPool(Application::GetGUILayer()->GetImGuiDescriptorPool());
        alloc_info_ds.setDescriptorSetCount(1);
        alloc_info_ds.setSetLayouts(*layout);

        try
        {
            m_ImageDescriptorSet.emplace(std::move(device.allocateDescriptorSets(alloc_info_ds)[0]));
        }
        catch (const vk::SystemError& e)
        {
            BC_THROW("Texture2D::CreateTexture: Could Not Allocate Descriptor Set. Error: {}", e.what());
        }
        catch (const std::exception& e)
        {
            BC_THROW("Texture2D::CreateTexture: Failed to Allocate Descriptor Set. Error: {}", e.what());
        }

        vk::DescriptorImageInfo image_info_ds = {};
        image_info_ds.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
        image_info_ds.setImageView(*m_ImageView);
        image_info_ds.setSampler(*m_Sampler);

        vk::WriteDescriptorSet write_ds = {};
        write_ds.setDstSet(*m_ImageDescriptorSet);
        write_ds.setDstBinding(0);
        write_ds.setDstArrayElement(0);
        write_ds.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
        write_ds.setDescriptorCount(1);
        write_ds.setPImageInfo(&image_info_ds);

        device.updateDescriptorSets(write_ds, nullptr);
    }

    std::optional<vk::raii::DescriptorSet>& Texture2D::GetImGuiDescriptorSet()
    {
        if (m_ImageDescriptorSet.has_value())
            return m_ImageDescriptorSet;

        auto& device = Application::GetVulkanCore()->GetLogicalDevice();
        auto& layout = Application::GetGUILayer()->GetImGuiDescriptorSetLayout();

        vk::DescriptorSetAllocateInfo alloc_info_ds = {};
        alloc_info_ds.setDescriptorPool(Application::GetGUILayer()->GetImGuiDescriptorPool());
        alloc_info_ds.setDescriptorSetCount(1);
        alloc_info_ds.setPSetLayouts(&(*layout));

        try
        {
            m_ImageDescriptorSet.emplace(std::move(device.allocateDescriptorSets(alloc_info_ds)[0]));
        }
        catch (const vk::SystemError& e)
        {
            BC_THROW("Texture2D::GetDescriptorSet: Could Not Allocate Descriptor Set. Error: {}", e.what());
        }
        catch (const std::exception& e)
        {
            BC_THROW("Texture2D::GetDescriptorSet: Failed to Allocate Descriptor Set. Error: {}", e.what());
        }

        vk::DescriptorImageInfo image_info_ds = {};
        image_info_ds.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
        image_info_ds.setImageView(*m_ImageView);
        image_info_ds.setSampler(*m_Sampler);

        vk::WriteDescriptorSet write_ds = {};
        write_ds.setDstSet(*m_ImageDescriptorSet);
        write_ds.setDstBinding(0);
        write_ds.setDstArrayElement(0);
        write_ds.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
        write_ds.setDescriptorCount(1);
        write_ds.setPImageInfo(&image_info_ds);

        device.updateDescriptorSets(write_ds, {});

        return m_ImageDescriptorSet;
    }

}