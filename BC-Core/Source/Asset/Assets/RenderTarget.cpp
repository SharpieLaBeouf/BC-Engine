#include "BC_PCH.h"
#include "RenderTarget.h"

namespace BC
{

    RenderTarget::RenderTarget(const RenderTargetSpecification& specification)
    {

        m_Specification = specification;

        Invalidate();
    }

    bool RenderTarget::IsValid() const
    {
        if (m_Specification.width == 0 || m_Specification.height == 0)
            return false;

        for (const auto& attachment : m_ColourAttachments)
        {
            if (!attachment.resolved_image || !attachment.resolved_image_view)
                return false;

            if (m_Specification.samples > 1 && (!attachment.multisampled_image || !attachment.multisampled_image_view))
                return false;
        }

        if (m_DepthStencilAttachment.specification.attachment_format != RenderTargetAttachmentFormat::None)
        {
            if (!m_DepthStencilAttachment.resolved_image || !m_DepthStencilAttachment.resolved_image_view)
                return false;

            if (m_Specification.samples > 1 && (!m_DepthStencilAttachment.multisampled_image || !m_DepthStencilAttachment.multisampled_image_view))
                return false;
        }

        return true;
    }

    void RenderTarget::Resize(uint32_t width, uint32_t height)
    {
        m_Specification.width = glm::max<uint32_t>(1, width);
        m_Specification.height = glm::max<uint32_t>(1, height);

        Invalidate();
    }

    VkImage RenderTarget::GetColourAttachmentImage(uint32_t index, bool get_multisampled_image) const
    {
        if (index < 0 || index >= m_ColourAttachments.size())
            return VK_NULL_HANDLE;

        if (IsMultiSampled() && get_multisampled_image)
        {
            return m_ColourAttachments[index].multisampled_image;
        }

        return m_ColourAttachments[index].resolved_image;
    }

    VkImageView RenderTarget::GetColourAttachmentView(uint32_t index, bool get_multisampled_image) const
    {
        if (index < 0 || index >= m_ColourAttachments.size())
            return VK_NULL_HANDLE;

        if (IsMultiSampled() && get_multisampled_image)
        {
            return m_ColourAttachments[index].multisampled_image_view;
        }

        return m_ColourAttachments[index].resolved_image_view;
    }

    VkImage RenderTarget::GetDepthAttachmentImage(bool get_multisampled_image) const
    {
        if (IsMultiSampled() && get_multisampled_image)
        {
            return m_DepthStencilAttachment.multisampled_image;
        }

        return m_DepthStencilAttachment.resolved_image;
    }

    VkImageView RenderTarget::GetDepthAttachmentView(bool get_multisampled_image) const
    {
        if (IsMultiSampled() && get_multisampled_image)
        {
            return m_DepthStencilAttachment.multisampled_image_view;
        }

        return m_DepthStencilAttachment.resolved_image_view;
    }

    const VkImageView* RenderTarget::FindAttachment(RenderTargetAttachmentFormat format) const
    {
        return nullptr;
    }

    uint32_t RenderTarget::FindAttachmentIndex(RenderTargetAttachmentFormat format) const
    {
        return 0;
    }

    VkDescriptorSet RenderTarget::RenderTargetAttachment::GetDescriptor() const
    {
        if (descriptor_set != VK_NULL_HANDLE)
            return descriptor_set;

        VkDevice device = Application::GetVulkanCore()->GetLogicalDevice();

        VkDescriptorSetAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorPool = Application::GetGUILayer()->GetImGuiDescriptorPool();
        alloc_info.descriptorSetCount = 1;

        auto descriptor_set_layout = Application::GetGUILayer()->GetImGuiDescriptorSetLayout();
        alloc_info.pSetLayouts = &descriptor_set_layout;

        BC_THROW(vkAllocateDescriptorSets(device, &alloc_info, &descriptor_set) == VK_SUCCESS,
            "RenderTarget::RenderTargetAttachment::GetDescriptor: Failed to allocate descriptor set.");

        VkDescriptorImageInfo image_info{};
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_info.imageView = resolved_image_view;
        image_info.sampler = Application::GetGUILayer()->GetImGuiDefaultSampler();

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = descriptor_set;
        write.dstBinding = 0;
        write.dstArrayElement = 0;
        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write.descriptorCount = 1;
        write.pImageInfo = &image_info;

        vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);

        return descriptor_set;
    }

    void RenderTarget::Invalidate()
    {
        BC_THROW
        (
            m_Specification.width > 0 && m_Specification.height > 0, 
            std::format("RenderTarget::RenderTarget: Could Not Initialise Render Target With 0 Extent: X:{}, Y:{}", 
                m_Specification.width, 
                m_Specification.height)
        );

        bool is_multisampled = IsMultiSampled();

        Cleanup();

        for (const auto& specification : m_Specification.attachments)
        {
            RenderTargetAttachment attachment = {};
            attachment.specification = specification;

            VkFormat format;
            VkImageAspectFlags aspect_mask;

            switch (specification.attachment_format)
            {
                case RenderTargetAttachmentFormat::RGBA8:
                {
                    format = VK_FORMAT_R8G8B8A8_UNORM;
                    aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT;
                    break;
                }
                case RenderTargetAttachmentFormat::RED_INTEGER:
                {
                    format = VK_FORMAT_R32_SINT;
                    aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT;
                    break;
                }
                case RenderTargetAttachmentFormat::DEPTH24STENCIL8:
                {
                    format = VK_FORMAT_D24_UNORM_S8_UINT;
                    aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
                    break;
                }
            }

            if (specification.attachment_format == RenderTargetAttachmentFormat::DEPTH24STENCIL8)
            {
                CreateAttachment
                (
                    format, 
                    aspect_mask, 
                    is_multisampled, 
                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT, 
                    attachment
                );
                m_DepthStencilAttachment = attachment;
            }
            else
            {
                CreateAttachment
                (
                    format, 
                    aspect_mask, 
                    is_multisampled, 
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT, 
                    attachment
                );
                m_ColourAttachments.push_back(attachment);
            }
        }
    }

    void RenderTarget::Cleanup()
    {
        auto device = Application::GetVulkanCore()->GetLogicalDevice();
        auto imgui_pool = Application::GetGUILayer()->GetImGuiDescriptorPool();

        auto destroy_attachment = [&](RenderTargetAttachment& attachment)
        {
            if (attachment.descriptor_set != VK_NULL_HANDLE)
            {
                vkFreeDescriptorSets(device, imgui_pool, 1, &attachment.descriptor_set);
                attachment.descriptor_set = VK_NULL_HANDLE;
            }

            if (attachment.multisampled_image_view) 
            {
                vkDestroyImageView(device, attachment.multisampled_image_view, nullptr);
                attachment.multisampled_image_view = VK_NULL_HANDLE;
            }

            if (attachment.multisampled_image) 
            {
                vkDestroyImage(device, attachment.multisampled_image, nullptr);
                attachment.multisampled_image = VK_NULL_HANDLE;
            }

            if (attachment.multisampled_memory) 
            {
                vkFreeMemory(device, attachment.multisampled_memory, nullptr);
                attachment.multisampled_memory = VK_NULL_HANDLE;
            }

            if (attachment.resolved_image_view) 
            {
                vkDestroyImageView(device, attachment.resolved_image_view, nullptr);
                attachment.resolved_image_view = VK_NULL_HANDLE;
            }

            if (attachment.resolved_image) 
            {
                vkDestroyImage(device, attachment.resolved_image, nullptr);
                attachment.resolved_image = VK_NULL_HANDLE;
            }

            if (attachment.resolved_memory) 
            {
                vkFreeMemory(device, attachment.resolved_memory, nullptr);
                attachment.resolved_memory = VK_NULL_HANDLE;
            }

            attachment.specification.attachment_format = RenderTargetAttachmentFormat::None;
        };

        for (auto& attachment : m_ColourAttachments)
            destroy_attachment(attachment);

        m_ColourAttachments.clear();

        destroy_attachment(m_DepthStencilAttachment);
    }

    void RenderTarget::CreateAttachment(VkFormat format, VkImageAspectFlags aspect, bool multisampled, VkImageUsageFlags usage, RenderTargetAttachment &out_attachment)
    {
        VkDevice device = Application::GetVulkanCore()->GetLogicalDevice();
        VkPhysicalDevice physical_device = Application::GetVulkanCore()->GetPhysicalDevice();

        const uint32_t width = m_Specification.width;
        const uint32_t height = m_Specification.height;
        const uint32_t samples = m_Specification.samples;

        auto create_image_and_view = [&](VkImage& image, VkImageView& view, VkDeviceMemory& memory, VkSampleCountFlagBits sample_count)
        {
            VkImageCreateInfo image_info{};
            image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            image_info.imageType = VK_IMAGE_TYPE_2D;
            image_info.format = format;
            image_info.extent.width = width;
            image_info.extent.height = height;
            image_info.extent.depth = 1;
            image_info.mipLevels = 1;
            image_info.arrayLayers = 1;
            image_info.samples = sample_count;
            image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
            image_info.usage = usage;

            image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

            BC_THROW(vkCreateImage(device, &image_info, nullptr, &image) == VK_SUCCESS, "RenderTarget::CreateAttachment: Could Not Create Image.");

            VkMemoryRequirements mem_requirements;
            vkGetImageMemoryRequirements(device, image, &mem_requirements);

            VkMemoryAllocateInfo alloc_info{};
            alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            alloc_info.allocationSize = mem_requirements.size;
            alloc_info.memoryTypeIndex = Application::GetVulkanCore()->FindMemoryType(mem_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            BC_THROW(vkAllocateMemory(device, &alloc_info, nullptr, &memory) == VK_SUCCESS, "RenderTarget::CreateAttachment: Could Not Allocate Image Memory.");
            BC_THROW(vkBindImageMemory(device, image, memory, 0) == VK_SUCCESS, "RenderTarget::CreateAttachment: Could Not Bind Image Memory.");

            VkImageViewCreateInfo view_info{};
            view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            view_info.image = image;
            view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            view_info.format = format;
            view_info.subresourceRange.aspectMask = aspect;
            view_info.subresourceRange.baseMipLevel = 0;
            view_info.subresourceRange.levelCount = 1;
            view_info.subresourceRange.baseArrayLayer = 0;
            view_info.subresourceRange.layerCount = 1;

            BC_THROW(vkCreateImageView(device, &view_info, nullptr, &view) == VK_SUCCESS, "RenderTarget::CreateAttachment: Could Not Create Image View.");
        };

        if (multisampled)
        {
            create_image_and_view(out_attachment.multisampled_image, out_attachment.multisampled_image_view, out_attachment.multisampled_memory, static_cast<VkSampleCountFlagBits>(samples));
            create_image_and_view(out_attachment.resolved_image, out_attachment.resolved_image_view, out_attachment.resolved_memory, VK_SAMPLE_COUNT_1_BIT);
        }
        else
        {
            create_image_and_view(out_attachment.resolved_image, out_attachment.resolved_image_view, out_attachment.resolved_memory, VK_SAMPLE_COUNT_1_BIT);
        }
    }

    std::shared_ptr<RenderTarget> RenderTarget::CreateRenderTarget(const RenderTargetSpecification &specification)
    {
        BC_CATCH_BEGIN();

        auto render_target = std::make_shared<RenderTarget>(specification);

        if (!render_target->IsValid())
            return nullptr;

        return render_target;

        BC_CATCH_END_RETURN(nullptr);
    }

}