#pragma once

// Core Headers
#include "Asset/Asset.h"

// C++ Standard Library Headers

// External Vendor Library Headers
#include <vulkan/vulkan.h>

namespace BC
{

    struct Texture2DSpecification
    {
        uint32_t width = 1;
        uint32_t height = 1;

        VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

        bool generate_mips = false;
        uint32_t mip_levels = 1;

        VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
        VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;

        VkFilter min_filter = VK_FILTER_LINEAR;
        VkFilter mag_filter = VK_FILTER_LINEAR;
        VkSamplerAddressMode address_mode_u = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        VkSamplerAddressMode address_mode_v = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        bool is_srgb = false;
        bool is_storage = false;
        bool is_render_target = false;
        bool flip_vertically = true;

        bool anisotropy_enabled = true;
    };

    class Texture2D : public Asset
    {

    public:

        AssetType GetType() const override { return AssetType::Texture2D; }

        Texture2D() = default;
        ~Texture2D();

        Texture2D(const Texture2D&) = delete;
        Texture2D(Texture2D&&) = default;

        Texture2D& operator=(const Texture2D&) = delete;
        Texture2D& operator=(Texture2D&&) = default;

        VkDescriptorSet GetDescriptor();

        // ---- Getters ----
        const Texture2DSpecification& GetSpecification() const { return m_Specification; }

        uint32_t GetWidth() const { return m_Specification.width; }
        uint32_t GetHeight() const { return m_Specification.height; }
        VkFormat GetFormat() const { return m_Specification.format; }
        
        bool ShouldGenerateMips() const { return m_Specification.generate_mips; }
        uint32_t GetMipLevels() const { return m_Specification.mip_levels; }

        bool IsSRGB() const { return m_Specification.is_srgb; }
        bool IsStorage() const { return m_Specification.is_storage; }
        bool IsRenderTarget() const { return m_Specification.is_render_target; }
        bool ShouldFlipVertically() const { return m_Specification.flip_vertically; }
        
        // ---- Setters ----
        void SetWidth(uint32_t width);
        void SetHeight(uint32_t height);
        void SetFormat(VkFormat format);

        void SetGenerateMips(bool value);
        void SetMipLevels(uint32_t levels);

        void SetSRGB(bool value);
        void SetStorage(bool value);
        void SetRenderTarget(bool value);
        void SetFlipVertically(bool value);

        // ---- Static ----
        static uint32_t GetChannelsFromFormat(VkFormat format);
        static uint32_t GetBytesPerChannel(VkFormat format);
        static std::shared_ptr<Texture2D> CreateTexture(const std::filesystem::path& texture_path, bool cache_data_cpu = false);
        static std::shared_ptr<Texture2D> CreateTexture(const Texture2DSpecification& specification, const unsigned char* texture_data_in = nullptr, VkFormat texture_data_in_format = VK_FORMAT_UNDEFINED, bool cache_data_cpu = false);

    private:

        void CreateTexture(const unsigned char* texture_data_in, VkFormat texture_data_in_format);

    private:

        VkFormat m_CachedDataFormat;
        std::vector<unsigned char> m_CachedData = {};

        Texture2DSpecification m_Specification;

        VkImage m_Image                     = VK_NULL_HANDLE;
        VkImageView m_ImageView             = VK_NULL_HANDLE;
        VkDeviceMemory m_ImageMemory        = VK_NULL_HANDLE;

        VkSampler m_Sampler                 = VK_NULL_HANDLE;
        VkDescriptorSet m_ImageDescriptor   = VK_NULL_HANDLE;

    };

}