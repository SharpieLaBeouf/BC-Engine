#pragma once

// Core Headers
#include "VulkanUtil.h"

// C++ Standard Library Headers
#include <vector>

// External Vendor Library Headers
#include <vulkan/vulkan.h>

namespace BC
{

    struct SwapchainSpecification
    {
        uint8_t ImageCount;
        VkSurfaceFormatKHR ImageFormat;
        VkPresentModeKHR PresentMode;
        VkExtent2D Extent;
    };

    struct SwapChainSupport
    {
        VkSurfaceCapabilitiesKHR Capabilities;
        std::vector<VkSurfaceFormatKHR> Formats;
        std::vector<VkPresentModeKHR> PresentModes;
    };

    class Swapchain
    {

    public:

        Swapchain() = delete;
        Swapchain(const SwapchainSpecification& swapchain_spec);
        ~Swapchain();

        void CleanUp();
        void Invalidate(const SwapchainSpecification& swapchain_spec);

        VkSwapchainKHR GetSwapchain() const { return m_Swapchain; }
        VkRenderPass GetRenderPass() const { return m_RenderPass; }
        uint32_t GetImageCount() const { return m_SwapChainImages.size(); }
        const std::vector<VkImage>& GetImages() const { return m_SwapChainImages; }
        const std::vector<VkImageView>& GetImageViews() const { return m_SwapChainImageViews; }
        const SwapchainSpecification& GetSpecification() const { return m_Specification; }
        const std::vector<VkFramebuffer>& GetFramebuffers() const { return m_SwapChainFramebuffers; }

        static SwapChainSupport GetSwapchainSupport(VkPhysicalDevice physical_device, VkSurfaceKHR surface);
        static VkSurfaceFormatKHR ChooseSwapchainFormat(const std::vector<VkSurfaceFormatKHR>& formats);
        static VkPresentModeKHR ChooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& present_modes);
        static VkExtent2D ChooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& capabilities);

        static uint8_t s_MinImageCount;

    private:

        SwapchainSpecification m_Specification;

        VkSwapchainKHR m_Swapchain = VK_NULL_HANDLE;
        VkRenderPass m_RenderPass = VK_NULL_HANDLE;

        std::vector<VkImage> m_SwapChainImages;
        std::vector<VkImageView> m_SwapChainImageViews;

        std::vector<VkFramebuffer> m_SwapChainFramebuffers;


    };

}