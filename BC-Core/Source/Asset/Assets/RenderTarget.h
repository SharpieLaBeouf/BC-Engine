#pragma once

// Core Headers
#include "Asset/Asset.h"

// C++ Standard Library Headers
#include <vector>
#include <memory>
#include <optional>

// External Vendor Library Headers
#include <vulkan/vulkan_raii.hpp>
#if defined(BC_PLATFORM_WINDOWS)
#include <vma/vk_mem_alloc.h>
#elif defined(BC_PLATFORM_LINUX)
#include <vk_mem_alloc.h>
#endif

namespace BC
{
	enum class RenderTargetAttachmentFormat : uint8_t
	{
		None = 0,

		// Colour
		RGBA8,
		RED_INTEGER,

		// Depth/stencil
		DEPTH24STENCIL8,

		// Defaults
        Colour = RGBA8,
		Depth = DEPTH24STENCIL8
	};

    struct RenderTargetAttachmentSpecification
    {
		RenderTargetAttachmentSpecification() = default;
		RenderTargetAttachmentSpecification(RenderTargetAttachmentFormat format)
			: attachment_format(format) {}

		RenderTargetAttachmentFormat attachment_format = RenderTargetAttachmentFormat::None;
    };

    struct RenderTargetSpecification
    {
        uint32_t width = 1, height = 1;
        uint32_t samples = 1;

        bool is_swapchain_target = false;

        std::vector<RenderTargetAttachmentSpecification> attachments;
    };


    class RenderTarget : public Asset
    {

        struct RenderTargetAttachment;

    public:

        RenderTarget(const RenderTargetSpecification& specification);
        ~RenderTarget() { Cleanup(); }

        AssetType GetType() const override { return AssetType::RenderTarget; }

        // ---- General Methods ----
        bool IsValid() const;
        bool IsMultiSampled() const { return m_Specification.samples > 1; }

        // ---- Setters ----
        void Resize(uint32_t width, uint32_t height);

        // ---- Getters ----
        vk::Extent2D GetExtent() const { return { m_Specification.width, m_Specification.height }; }
        
        vk::Image GetColourAttachmentImage(uint32_t index, bool get_multisampled_image = false) const;
        std::optional<std::reference_wrapper<vk::raii::ImageView>> GetColourAttachmentView(uint32_t index, bool get_multisampled_image = false);

        vk::Image GetDepthAttachmentImage(bool get_multisampled_image = false) const;
        std::optional<std::reference_wrapper<vk::raii::ImageView>> GetDepthAttachmentView(bool get_multisampled_image = false);

        const std::vector<RenderTargetAttachment>& GetAttachments() const { return m_ColourAttachments; }
        const RenderTargetAttachment& GetDepthStencilAttachment() const { return m_DepthStencilAttachment; }

        // ---- Static Methods ----
        static std::shared_ptr<RenderTarget> CreateRenderTarget(const RenderTargetSpecification& specification);

    private:

        void Invalidate();
        void Cleanup();

        void CreateAttachment(vk::Format format, vk::ImageAspectFlags aspect, bool multisampled, vk::ImageUsageFlags usage, RenderTargetAttachment& out_attachment);

        struct RenderTargetAttachment
        {
            RenderTargetAttachmentSpecification specification = { RenderTargetAttachmentFormat::None };

            vk::Image resolved_image;
            VmaAllocation resolved_allocation = VK_NULL_HANDLE;
            std::optional<vk::raii::ImageView> resolved_image_view;

            vk::Image multisampled_image;
            VmaAllocation multisampled_allocation = VK_NULL_HANDLE;
            std::optional<vk::raii::ImageView> multisampled_image_view;

            mutable std::optional<vk::raii::DescriptorSet> descriptor_set;

            vk::raii::DescriptorSet& GetImGuiDescriptorSet();
        };
        
        RenderTargetSpecification m_Specification;
        std::vector<RenderTargetAttachment> m_ColourAttachments;
        RenderTargetAttachment m_DepthStencilAttachment;
    };

}