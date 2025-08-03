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

    vk::Image RenderTarget::GetColourAttachmentImage(uint32_t index, bool get_multisampled_image) const
    {
        if (index >= m_ColourAttachments.size())
            return VK_NULL_HANDLE;

        if (IsMultiSampled() && get_multisampled_image)
        {
            return m_ColourAttachments[index].multisampled_image;
        }

        return m_ColourAttachments[index].resolved_image;
    }

    std::optional<std::reference_wrapper<vk::raii::ImageView>> RenderTarget::GetColourAttachmentView(uint32_t index, bool get_multisampled_image)
    {
        if (index >= m_ColourAttachments.size())
            return std::nullopt;

        if (IsMultiSampled() && get_multisampled_image)
        {
            if (!m_ColourAttachments[index].multisampled_image_view.has_value())
            {
                return std::nullopt;
            }

            return std::ref(*m_ColourAttachments[index].multisampled_image_view);
        }

        if (!m_ColourAttachments[index].resolved_image_view.has_value())
        {
            return std::nullopt;
        }

        return std::ref(*m_ColourAttachments[index].resolved_image_view);
    }

    vk::Image RenderTarget::GetDepthAttachmentImage(bool get_multisampled_image) const
    {
        if (IsMultiSampled() && get_multisampled_image)
        {
            return m_DepthStencilAttachment.multisampled_image;
        }

        return m_DepthStencilAttachment.resolved_image;
    }

    std::optional<std::reference_wrapper<vk::raii::ImageView>> RenderTarget::GetDepthAttachmentView(bool get_multisampled_image)
    {
        if (IsMultiSampled() && get_multisampled_image)
        {
            return std::ref(*m_DepthStencilAttachment.multisampled_image_view);
        }

        return std::ref(*m_DepthStencilAttachment.resolved_image_view);
    }

    vk::raii::DescriptorSet& RenderTarget::RenderTargetAttachment::GetImGuiDescriptorSet()
    {
        if (descriptor_set)
            return *descriptor_set;

        auto& device = Application::GetVulkanCore()->GetLogicalDevice();

        vk::DescriptorSetAllocateInfo alloc_info = {};
        alloc_info.setDescriptorPool(Application::GetGUILayer()->GetImGuiDescriptorPool());
        alloc_info.setDescriptorSetCount(1);
        alloc_info.setPSetLayouts(&(*Application::GetGUILayer()->GetImGuiDescriptorSetLayout()));

        try
        {
            descriptor_set.emplace(std::move(device.allocateDescriptorSets(alloc_info)[0]));
        }
        catch (const vk::SystemError& e)
        {
            BC_THROW("RenderTargetAttachment::GetDescriptorSet: Could Not Allocate Descriptor Set. Error: {}", e.what());
        }
        catch (const std::exception& e)
        {
            BC_THROW("RenderTargetAttachment::GetDescriptorSet: Failed to Allocate Descriptor Set. Error: {}", e.what());
        }

        vk::DescriptorImageInfo image_info = {};
        image_info.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
        image_info.setImageView(*resolved_image_view);
        image_info.setSampler(Application::GetGUILayer()->GetImGuiDefaultSampler());

        vk::WriteDescriptorSet write = {};
        write.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
        write.setDstSet(*descriptor_set);
        write.setDstBinding(0);
        write.setDstArrayElement(0);
        write.setDescriptorCount(1);
        write.setPImageInfo(&image_info);

        device.updateDescriptorSets(write, {});

        return *descriptor_set;
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

            vk::Format format;
            vk::ImageAspectFlags aspect_mask;

            switch (specification.attachment_format)
            {
                case RenderTargetAttachmentFormat::RGBA8:
                {
                    format = vk::Format::eR8G8B8A8Unorm;
                    aspect_mask = vk::ImageAspectFlagBits::eColor;
                    break;
                }
                case RenderTargetAttachmentFormat::RED_INTEGER:
                {
                    format = vk::Format::eR32Sint;
                    aspect_mask = vk::ImageAspectFlagBits::eColor;
                    break;
                }
                case RenderTargetAttachmentFormat::DEPTH24STENCIL8:
                {
                    format = vk::Format::eD24UnormS8Uint;
                    aspect_mask = vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
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
                    vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eStorage, 
                    attachment
                );
                m_DepthStencilAttachment = std::move(attachment);
            }
            else
            {
                CreateAttachment
                (
                    format, 
                    aspect_mask, 
                    is_multisampled, 
                    vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eStorage, 
                    attachment
                );
                m_ColourAttachments.push_back(std::move(attachment));
            }
        }
    }

    void RenderTarget::Cleanup()
    {
        auto& device = Application::GetVulkanCore()->GetLogicalDevice();
        auto vma_allocator = Application::GetVulkanCore()->GetAllocator();

        auto destroy_attachment = [&](RenderTargetAttachment& attachment)
        {
            attachment.descriptor_set.reset();
            attachment.multisampled_image_view.reset();
            attachment.resolved_image_view.reset();
            
            if (attachment.multisampled_image && attachment.multisampled_allocation)
            {
                vmaDestroyImage(vma_allocator, attachment.multisampled_image, attachment.multisampled_allocation);
                attachment.multisampled_image = VK_NULL_HANDLE;
                attachment.multisampled_allocation = nullptr;
            }

            if (attachment.resolved_image && attachment.resolved_allocation)
            {
                vmaDestroyImage(vma_allocator, attachment.resolved_image, attachment.resolved_allocation);
                attachment.resolved_image = VK_NULL_HANDLE;
                attachment.resolved_allocation = nullptr;
            }

            attachment.specification.attachment_format = RenderTargetAttachmentFormat::None;
        };

        for (auto& attachment : m_ColourAttachments)
            destroy_attachment(attachment);

        m_ColourAttachments.clear();

        destroy_attachment(m_DepthStencilAttachment);
    }

    void RenderTarget::CreateAttachment(
        vk::Format format,
        vk::ImageAspectFlags aspect,
        bool multisampled,
        vk::ImageUsageFlags usage,
        RenderTargetAttachment& out_attachment)
    {
        auto& device = Application::GetVulkanCore()->GetLogicalDevice();
        auto vma_allocator = Application::GetVulkanCore()->GetAllocator();

        const uint32_t width = m_Specification.width;
        const uint32_t height = m_Specification.height;
        const uint32_t samples = m_Specification.samples;

        auto create_image_and_view = [&](vk::Image& image, VmaAllocation& allocation, std::optional<vk::raii::ImageView>& image_view, vk::SampleCountFlagBits sample_count)
        {
            vk::ImageCreateInfo image_info = {};
            image_info.setImageType(vk::ImageType::e2D);
            image_info.setFormat(format);
            image_info.setExtent({ width, height, 1 });
            image_info.setMipLevels(1);
            image_info.setArrayLayers(1);
            image_info.setSamples(sample_count);
            image_info.setTiling(vk::ImageTiling::eOptimal);
            image_info.setUsage(static_cast<vk::ImageUsageFlags>(usage));
            image_info.setSharingMode(vk::SharingMode::eExclusive);
            image_info.setInitialLayout(vk::ImageLayout::eUndefined);

            VmaAllocationCreateInfo alloc_info{};
            alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
            
            VkImage raw_image = VK_NULL_HANDLE;

            BC_THROW(
                vmaCreateImage(
                vma_allocator,
                image_info,
                &alloc_info,
                &raw_image,
                &allocation,
                nullptr
                ) == VK_SUCCESS,
                "RenderTarget::CreateAttachment: Could Not Create Image."
            );

            image = vk::Image(raw_image);

            vk::ImageViewCreateInfo view_info = {};
            view_info.setImage(image);
            view_info.setViewType(vk::ImageViewType::e2D);
            view_info.setFormat(format);
            view_info.subresourceRange.setAspectMask(aspect);
            view_info.subresourceRange.setBaseMipLevel(0);
            view_info.subresourceRange.setLevelCount(1);
            view_info.subresourceRange.setBaseArrayLayer(0);
            view_info.subresourceRange.setLayerCount(1);

            image_view.emplace(device, view_info);
        };

        create_image_and_view(
            out_attachment.resolved_image,
            out_attachment.resolved_allocation,
            out_attachment.resolved_image_view,
            vk::SampleCountFlagBits::e1
        );

        if (multisampled)
        {
            create_image_and_view(
                out_attachment.multisampled_image,
                out_attachment.multisampled_allocation,
                out_attachment.multisampled_image_view,
                static_cast<vk::SampleCountFlagBits>(samples)
            );
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