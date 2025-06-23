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
        auto vulkan_core = Application::GetVulkanCore();

        // Destroy Framebuffers
        for (auto& framebuffer : m_SwapChainFramebuffers)
        {
            if (framebuffer)
                vkDestroyFramebuffer(vulkan_core->GetLogicalDevice(), framebuffer, nullptr);
        }
        m_SwapChainFramebuffers.clear();

        // Destroy RenderPass
        if (m_RenderPass)
        {
            vkDestroyRenderPass(vulkan_core->GetLogicalDevice(), m_RenderPass, nullptr);
            m_RenderPass = VK_NULL_HANDLE;
        }

        // Destroy Image Views
        for (auto& image_view : m_SwapChainImageViews)
        {
            if (image_view)
            {
                vkDestroyImageView(vulkan_core->GetLogicalDevice(), image_view, VK_NULL_HANDLE);
            }
        }
        m_SwapChainImageViews.clear();

        // Destroy Swapchain
        if (m_Swapchain)
        {
            vkDestroySwapchainKHR(vulkan_core->GetLogicalDevice(), m_Swapchain, VK_NULL_HANDLE);
            m_Swapchain = VK_NULL_HANDLE;
        }
        m_SwapChainImages.clear();
    }

    void Swapchain::Invalidate(const SwapchainSpecification& swapchain_spec)
    {
        CleanUp();

        m_Specification = swapchain_spec;

        auto vulkan_core = Application::GetVulkanCore();
        uint32_t queue_family_indices[] = { vulkan_core->GetGraphicsQueueFamily(), vulkan_core->GetComputeQueueFamily(), vulkan_core->GetPresentQueueFamily() };

        VkSwapchainCreateInfoKHR swap_info = {};
        swap_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swap_info.pNext = VK_NULL_HANDLE;
        swap_info.flags = 0;
        swap_info.surface = vulkan_core->GetSurface();
        swap_info.minImageCount = m_Specification.ImageCount;
        swap_info.imageFormat = m_Specification.ImageFormat.format;
        swap_info.imageColorSpace = m_Specification.ImageFormat.colorSpace;
        swap_info.imageExtent = m_Specification.Extent;
        swap_info.imageArrayLayers = 1;
        swap_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swap_info.imageSharingMode = (vulkan_core->GetGraphicsQueueFamily() != vulkan_core->GetPresentQueueFamily()) ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
        swap_info.queueFamilyIndexCount = (vulkan_core->GetGraphicsQueueFamily() != vulkan_core->GetPresentQueueFamily()) ? 2U : 0U;
        swap_info.pQueueFamilyIndices = (vulkan_core->GetGraphicsQueueFamily() != vulkan_core->GetPresentQueueFamily()) ? queue_family_indices : VK_NULL_HANDLE;
        swap_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        swap_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swap_info.presentMode = m_Specification.PresentMode;
        swap_info.clipped = VK_TRUE;
        swap_info.oldSwapchain = VK_NULL_HANDLE;

        VkResult result = vkCreateSwapchainKHR(vulkan_core->GetLogicalDevice(), &swap_info, VK_NULL_HANDLE, &m_Swapchain);
        BC_THROW(result == VK_SUCCESS, "Swapchain::Invalidate: Could Not Create SwapchainKHR.");

        uint32_t image_count = 0;
        result = vkGetSwapchainImagesKHR(vulkan_core->GetLogicalDevice(), m_Swapchain, &image_count, nullptr);

        BC_THROW(result == VK_SUCCESS, "Swapchain::Invalidate: Could Not Get SwapChain Image Count.");
        BC_THROW(image_count > 0,      "Swapchain::Invalidate: Image Count Not Valid.");

        m_SwapChainImages.resize(image_count);
        result = vkGetSwapchainImagesKHR(vulkan_core->GetLogicalDevice(), m_Swapchain, &image_count, m_SwapChainImages.data());

        BC_THROW(result == VK_SUCCESS, "Swapchain::Invalidate: Could Not Get SwapChain Images.");

        m_SwapChainImageViews.resize(m_SwapChainImages.size());
        for (auto i = 0; i < m_SwapChainImageViews.size(); ++i)
        {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = m_SwapChainImages[i];

            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = m_Specification.ImageFormat.format;

            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;
            
            result = vkCreateImageView(vulkan_core->GetLogicalDevice(), &createInfo, nullptr, &m_SwapChainImageViews[i]);
            BC_THROW(result == VK_SUCCESS, "VulkanCore::CreateSwapchain: Failed to Create Image View.");
        }

        // Create RenderPass
        VkAttachmentDescription color_attachment = {};
        color_attachment.format = m_Specification.ImageFormat.format;
        color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Clear on Load
        color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // Store on Finish
        color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference color_attachment_ref = {};
        color_attachment_ref.attachment = 0;
        color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_attachment_ref;

        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo render_pass_info = {};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_info.attachmentCount = 1;
        render_pass_info.pAttachments = &color_attachment;
        render_pass_info.subpassCount = 1;
        render_pass_info.pSubpasses = &subpass;
        render_pass_info.dependencyCount = 1;
        render_pass_info.pDependencies = &dependency;

        result = vkCreateRenderPass(vulkan_core->GetLogicalDevice(), &render_pass_info, nullptr, &m_RenderPass);

        BC_THROW(result == VK_SUCCESS, "Swapchain::Invalidate: Failed to Create Render Pass.");

        m_SwapChainFramebuffers.resize(m_SwapChainImageViews.size());
        for (size_t i = 0; i < m_SwapChainImageViews.size(); ++i)
        {
            VkImageView attachments[] = { m_SwapChainImageViews[i] };

            VkFramebufferCreateInfo framebuffer_info{};
            framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebuffer_info.renderPass = m_RenderPass;
            framebuffer_info.attachmentCount = 1;
            framebuffer_info.pAttachments = attachments;
            framebuffer_info.width = m_Specification.Extent.width;
            framebuffer_info.height = m_Specification.Extent.height;
            framebuffer_info.layers = 1;

            result = vkCreateFramebuffer(vulkan_core->GetLogicalDevice(), &framebuffer_info, nullptr, &m_SwapChainFramebuffers[i]);
            BC_THROW(result == VK_SUCCESS, "Swapchain::Invalidate: Failed to Create Framebuffer.");
        }
    }

    SwapChainSupport Swapchain::GetSwapchainSupport(VkPhysicalDevice physical_device, VkSurfaceKHR surface)
    {
        SwapChainSupport support;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &support.Capabilities);

        uint32_t format_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr);

        if (format_count != 0) 
        {
            support.Formats.resize(format_count);
            vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, support.Formats.data());
        }

        uint32_t present_mode_count;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, nullptr);

        if (present_mode_count != 0) 
        {
            support.PresentModes.resize(present_mode_count);
            vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, support.PresentModes.data());
        }

        return support;
    }

    VkSurfaceFormatKHR Swapchain::ChooseSwapchainFormat(const std::vector<VkSurfaceFormatKHR>& formats)
    {
        BC_ASSERT(!formats.empty(), "Swapchain::ChooseSwapchainFormat: No Available Formats.");

        for (const auto& available_format : formats) 
        {
            if (available_format.format == VK_FORMAT_B8G8R8A8_UNORM && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
            {
                return available_format;
            }
        }

        return formats[0];
    }

    VkPresentModeKHR Swapchain::ChooseSwapchainPresentMode(const std::vector<VkPresentModeKHR> &present_modes)
    {
        // Choose Mailbox First if Available
        for (const auto& available_present_mode : present_modes) 
        {
            if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) 
            {
                return available_present_mode;
            }
        }

        // Default to First In First Out
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D Swapchain::ChooseSwapchainExtent(const VkSurfaceCapabilitiesKHR &capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
        {
            return capabilities.currentExtent;
        }

        auto actual_extent = Application::GetWindow()->GetSize();

        actual_extent.x = std::clamp(actual_extent.x, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actual_extent.y = std::clamp(actual_extent.y, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return VkExtent2D{ actual_extent.x, actual_extent.y };
    }
}