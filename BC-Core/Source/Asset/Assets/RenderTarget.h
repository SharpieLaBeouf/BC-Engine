#pragma once

// Core Headers
#include "Asset/Asset.h"

// C++ Standard Library Headers
#include <vector>
#include <memory>

// External Vendor Library Headers
#include <vulkan/vulkan.h>

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
        VkExtent2D GetExtent() const { return { m_Specification.width, m_Specification.height }; }
        
        VkImage GetColourAttachmentImage(uint32_t index, bool get_multisampled_image = false) const;
        VkImageView GetColourAttachmentView(uint32_t index, bool get_multisampled_image = false) const;
        
        VkImage GetDepthAttachmentImage(bool get_multisampled_image = false) const;
        VkImageView GetDepthAttachmentView(bool get_multisampled_image = false) const;

        const std::vector<RenderTargetAttachment>& GetAttachments() const { return m_ColourAttachments; }
        const RenderTargetAttachment& GetDepthStencilAttachment() const { return m_DepthStencilAttachment; }

        const VkImageView* FindAttachment(RenderTargetAttachmentFormat format) const;
        uint32_t FindAttachmentIndex(RenderTargetAttachmentFormat format) const;

        // ---- Static Methods ----
        static std::shared_ptr<RenderTarget> CreateRenderTarget(const RenderTargetSpecification& specification);

    private:

        void Invalidate();
        void Cleanup();

        void CreateAttachment(VkFormat format, VkImageAspectFlags aspect, bool multisampled, VkImageUsageFlags usage, RenderTargetAttachment& out_attachment);

        struct RenderTargetAttachment
        {
            RenderTargetAttachmentSpecification specification = { RenderTargetAttachmentFormat::None };

            VkImage resolved_image                  = VK_NULL_HANDLE;
            VkImageView resolved_image_view         = VK_NULL_HANDLE;

            VkImage multisampled_image              = VK_NULL_HANDLE;
            VkImageView multisampled_image_view     = VK_NULL_HANDLE;

            VkDeviceMemory multisampled_memory      = VK_NULL_HANDLE;
            VkDeviceMemory resolved_memory          = VK_NULL_HANDLE;

            mutable VkDescriptorSet descriptor_set          = VK_NULL_HANDLE;

            VkDescriptorSet GetDescriptor() const;
        };
        
        RenderTargetSpecification m_Specification;
        std::vector<RenderTargetAttachment> m_ColourAttachments;
        RenderTargetAttachment m_DepthStencilAttachment;
    };

}