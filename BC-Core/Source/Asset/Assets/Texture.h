#pragma once

// Core Headers
#include "Asset/Asset.h"

// C++ Standard Library Headers
#include <optional>

// External Vendor Library Headers
#include <vulkan/vulkan_raii.hpp>

namespace BC
{

    struct Texture2DSpecification
    {
        uint32_t width = 1;
        uint32_t height = 1;

        vk::Format format = vk::Format::eR8G8B8A8Unorm; // Default format

        bool generate_mips = false;
        uint32_t mip_levels = 1;

        vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
        vk::ImageTiling tiling = vk::ImageTiling::eOptimal;
        vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1;

        vk::Filter min_filter = vk::Filter::eLinear;
        vk::Filter mag_filter = vk::Filter::eLinear;
        vk::SamplerAddressMode address_mode_u = vk::SamplerAddressMode::eRepeat;
        vk::SamplerAddressMode address_mode_v = vk::SamplerAddressMode::eRepeat;

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

        std::optional<vk::raii::DescriptorSet>& GetImGuiDescriptorSet();

        // ---- Getters ----
        const Texture2DSpecification& GetSpecification() const { return m_Specification; }

        uint32_t GetWidth() const { return m_Specification.width; }
        uint32_t GetHeight() const { return m_Specification.height; }
        vk::Format GetFormat() const { return m_Specification.format; }
        
        bool ShouldGenerateMips() const { return m_Specification.generate_mips; }
        uint32_t GetMipLevels() const { return m_Specification.mip_levels; }

        bool IsSRGB() const { return m_Specification.is_srgb; }
        bool IsStorage() const { return m_Specification.is_storage; }
        bool IsRenderTarget() const { return m_Specification.is_render_target; }
        bool ShouldFlipVertically() const { return m_Specification.flip_vertically; }
        
        // ---- Setters ----
        void SetWidth(uint32_t width);
        void SetHeight(uint32_t height);
        void SetFormat(vk::Format format);

        void SetGenerateMips(bool value);
        void SetMipLevels(uint32_t levels);

        void SetSRGB(bool value);
        void SetStorage(bool value);
        void SetRenderTarget(bool value);
        void SetFlipVertically(bool value);

        // ---- Static ----
        static uint32_t GetChannelsFromFormat(vk::Format format);
        static uint32_t GetBytesPerChannel(vk::Format format);

        static std::shared_ptr<Texture2D> CreateTexture(const std::filesystem::path& texture_path, bool cache_data_cpu = false);
        static std::shared_ptr<Texture2D> CreateTexture(const Texture2DSpecification& specification, const unsigned char* texture_data_in = nullptr, vk::Format texture_data_in_format = vk::Format::eUndefined, bool cache_data_cpu = false);

        static std::shared_ptr<Texture2D> CreateTextureFromFileBytes(const std::vector<uint8_t>& texture_file_bytes, bool cache_cpu_data = false);

    private:

        void CreateTexture(const unsigned char* texture_data_in, vk::Format texture_data_in_format);

    private:

        vk::Format m_CachedDataFormat;
        std::vector<unsigned char> m_CachedData = {};

        Texture2DSpecification m_Specification;

        VmaAllocation m_ImageAllocation;
        vk::Image m_Image = VK_NULL_HANDLE;
        std::optional<vk::raii::ImageView> m_ImageView;

        std::optional<vk::raii::Sampler> m_Sampler;
        std::optional<vk::raii::DescriptorSet> m_ImageDescriptorSet;
    };

    class TextureCubeMap : public Asset
    {

    public:

        AssetType GetType() const override { return AssetType::Texture2D; }

        void SerialiseAsset(YAML::Emitter& out)
        {

        }

        void DeserialiseAsset(const YAML::Node& data)
        {

        }
        
        static std::shared_ptr<TextureCubeMap> CreateTextureCubeMap(const std::string& serialised_cube_map_data);

    };
}