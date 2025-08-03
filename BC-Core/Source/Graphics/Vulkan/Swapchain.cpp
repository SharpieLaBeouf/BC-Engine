#include "BC_PCH.h"
#include "Swapchain.h"

namespace BC
{
    uint8_t Swapchain::s_MinImageCount = 2;

    Swapchain::Swapchain(const SwapchainSpecification &swapchain_spec)
    {
        Invalidate(swapchain_spec);
    }

    Swapchain::~Swapchain()
    {
        CleanUp();
    }

    void Swapchain::CleanUp()
    {
        // Framebuffers
        m_SwapChainFramebuffers.clear();

        // RenderPass
        m_RenderPass.reset();

        // Image Views
        m_SwapChainImageViews.clear();

        // Swapchain
        m_Swapchain.reset();
        m_SwapChainImages.clear();
    }

    void Swapchain::Invalidate(const SwapchainSpecification& swapchain_spec)
    {
        CleanUp();

        m_Specification = swapchain_spec;

        auto vulkan_core = Application::GetVulkanCore();
        uint32_t queue_family_indices[] = { vulkan_core->GetGraphicsQueueFamily(), vulkan_core->GetComputeQueueFamily(), vulkan_core->GetPresentQueueFamily() };

        vk::SwapchainCreateInfoKHR swap_create_info;
        swap_create_info.setSurface(vulkan_core->GetSurface());
        swap_create_info.setMinImageCount(m_Specification.image_count);
        swap_create_info.setImageFormat(m_Specification.image_format.format);
        swap_create_info.setImageColorSpace(m_Specification.image_format.colorSpace);
        swap_create_info.setImageExtent(m_Specification.extent);
        swap_create_info.setImageArrayLayers(1);
        swap_create_info.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);
        swap_create_info.setImageSharingMode((vulkan_core->GetGraphicsQueueFamily() != vulkan_core->GetPresentQueueFamily()) ? vk::SharingMode::eConcurrent : vk::SharingMode::eExclusive);
        swap_create_info.setQueueFamilyIndexCount((vulkan_core->GetGraphicsQueueFamily() != vulkan_core->GetPresentQueueFamily()) ? 2U : 0U);
        swap_create_info.setPQueueFamilyIndices((vulkan_core->GetGraphicsQueueFamily() != vulkan_core->GetPresentQueueFamily()) ? queue_family_indices : nullptr);
        swap_create_info.setPreTransform(vk::SurfaceTransformFlagBitsKHR::eIdentity);
        swap_create_info.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
        swap_create_info.setPresentMode(m_Specification.present_mode);
        swap_create_info.setClipped(VK_TRUE);
        swap_create_info.setOldSwapchain(VK_NULL_HANDLE);
        
        try
        {
            m_Swapchain.emplace(vulkan_core->GetLogicalDevice(), swap_create_info);
        }
        catch (const vk::SystemError& e)
        {
            BC_THROW
            (
                static_cast<vk::Result>(e.code().value()) == vk::Result::eSuccess, 
                std::format("Swapchain::Invalidate: Could Not Create SwapchainKHR - {}.", e.what())
            );
        }

        try
        {
            m_SwapChainImages = m_Swapchain->getImages();
        }
        catch (const vk::SystemError& e)
        {
            BC_THROW
            (
                static_cast<vk::Result>(e.code().value()) == vk::Result::eSuccess, 
                std::format("Swapchain::Invalidate: Could Not Get Swapchain Images - {}.", e.what())
            );
        }

        m_SwapChainImageViews.resize(m_SwapChainImages.size());
        for (auto i = 0; i < m_SwapChainImageViews.size(); ++i)
        {
            try
            {
                vk::ImageViewCreateInfo image_view_create_info = {};

                image_view_create_info.setImage(m_SwapChainImages[i]);
                image_view_create_info.setViewType(vk::ImageViewType::e2D);
                image_view_create_info.setFormat(m_Specification.image_format.format);
                
                image_view_create_info.setComponents(vk::ComponentMapping()
                                                      .setR(vk::ComponentSwizzle::eIdentity)
                                                      .setG(vk::ComponentSwizzle::eIdentity)
                                                      .setB(vk::ComponentSwizzle::eIdentity)
                                                      .setA(vk::ComponentSwizzle::eIdentity));
                
                image_view_create_info.setSubresourceRange(
                           vk::ImageSubresourceRange()
                               .setAspectMask(vk::ImageAspectFlagBits::eColor)
                               .setBaseMipLevel(0)
                               .setLevelCount(1)
                               .setBaseArrayLayer(0)
                               .setLayerCount(1));

                m_SwapChainImageViews[i].emplace(std::move(vulkan_core->GetLogicalDevice().createImageView(image_view_create_info)));
            }
            catch (const vk::SystemError& e)
            {
                BC_THROW
                (
                    static_cast<vk::Result>(e.code().value()) == vk::Result::eSuccess, 
                    std::format("Swapchain::Invalidate: Failed to Create Image View - {}.", e.what())
                );
            }
        }

        vk::AttachmentDescription colour_attachment = {};
        colour_attachment.setFormat(m_Specification.image_format.format);
        colour_attachment.setSamples(vk::SampleCountFlagBits::e1);
        colour_attachment.setLoadOp(vk::AttachmentLoadOp::eClear); // Clear on Load
        colour_attachment.setStoreOp(vk::AttachmentStoreOp::eStore); // Store on Finish
        colour_attachment.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare);
        colour_attachment.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
        colour_attachment.setInitialLayout(vk::ImageLayout::eUndefined);
        colour_attachment.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

        vk::AttachmentReference colour_attachment_ref = {};
        colour_attachment_ref.setAttachment(0);
        colour_attachment_ref.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

        vk::SubpassDescription subpass = {};
        subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
        subpass.setColorAttachmentCount(1);
        subpass.setPColorAttachments(&colour_attachment_ref);

        vk::SubpassDependency dependency = {};
        dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL);
        dependency.setDstSubpass(0);
        dependency.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
        dependency.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
        dependency.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

        vk::RenderPassCreateInfo render_pass_info = {};
        render_pass_info.setAttachmentCount(1);
        render_pass_info.setPAttachments(&colour_attachment);
        render_pass_info.setSubpassCount(1);
        render_pass_info.setPSubpasses(&subpass);
        render_pass_info.setDependencyCount(1);
        render_pass_info.setPDependencies(&dependency);

        try
        {
            m_RenderPass.emplace(vulkan_core->GetLogicalDevice(), render_pass_info);
        }
        catch (const vk::SystemError& e)
        {
            BC_THROW
            (
                static_cast<vk::Result>(e.code().value()) == vk::Result::eSuccess, 
                std::format("Swapchain::Invalidate: Could Not Create Render Pass - {}.", e.what())
            );
        }

        m_SwapChainFramebuffers.resize(m_SwapChainImageViews.size());
        for (size_t i = 0; i < m_SwapChainImageViews.size(); ++i)
        {
            try
            {
                vk::FramebufferCreateInfo framebuffer_info;
                framebuffer_info.setRenderPass(*m_RenderPass);
                framebuffer_info.setAttachmentCount(1);

                vk::ImageView raw_view = *m_SwapChainImageViews[i];
                framebuffer_info.setPAttachments(&raw_view);

                framebuffer_info.setWidth(m_Specification.extent.width);
                framebuffer_info.setHeight(m_Specification.extent.height);
                framebuffer_info.setLayers(1);

                m_SwapChainFramebuffers[i].emplace(std::move(vulkan_core->GetLogicalDevice().createFramebuffer(framebuffer_info)));
            }
            catch (const vk::SystemError& e)
            {
                BC_THROW
                (
                    static_cast<vk::Result>(e.code().value()) == vk::Result::eSuccess, 
                    std::format("Swapchain::Invalidate: Could Not Create Framebuffer - {}.", e.what())
                );
            }
        }
    }

    SwapChainSupport Swapchain::GetSwapchainSupport(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface)
    {
        SwapChainSupport support;
        
        support.capabilities = physical_device.getSurfaceCapabilitiesKHR(surface);
        support.formats = physical_device.getSurfaceFormatsKHR(surface);
        support.present_modes = physical_device.getSurfacePresentModesKHR(surface);

        return support;
    }

    vk::SurfaceFormatKHR Swapchain::ChooseSwapchainFormat(const std::vector<vk::SurfaceFormatKHR>& formats)
    {
        BC_ASSERT(!formats.empty(), "Swapchain::ChooseSwapchainFormat: No Available Formats.");

        for (const auto& available_format : formats) 
        {
            if (available_format.format == vk::Format::eB8G8R8A8Unorm && 
                available_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) 
            {
                return available_format;
            }
        }

        return formats[0];
    }

    vk::PresentModeKHR Swapchain::ChooseSwapchainPresentMode(const std::vector<vk::PresentModeKHR> &present_modes)
    {
        // Choose Mailbox First if Available
        for (const auto& available_present_mode : present_modes) 
        {
            if (available_present_mode == vk::PresentModeKHR::eMailbox) 
            {
                return available_present_mode;
            }
        }

        // Default to First In First Out
        return vk::PresentModeKHR::eFifo;
    }

    vk::Extent2D Swapchain::ChooseSwapchainExtent(const vk::SurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
        {
            return capabilities.currentExtent;
        }

        auto actual_extent = Application::GetWindow()->GetSize();

        actual_extent.x = std::clamp(actual_extent.x, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actual_extent.y = std::clamp(actual_extent.y, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return vk::Extent2D{ actual_extent.x, actual_extent.y };
    }
}