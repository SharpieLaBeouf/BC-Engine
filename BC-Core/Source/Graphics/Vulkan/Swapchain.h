#pragma once

// Core Headers
#include "VulkanUtil.h"

// C++ Standard Library Headers
#include <vector>

// External Vendor Library Headers
#include <vulkan/vulkan_raii.hpp>

namespace BC
{

    struct SwapchainSpecification
    {
        uint8_t image_count;
        vk::SurfaceFormatKHR image_format;
        vk::PresentModeKHR present_mode;
        vk::Extent2D extent;
    };

    struct SwapChainSupport
    {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> present_modes;
    };

    class Swapchain
    {

    public:

        Swapchain() = delete;
        Swapchain(const SwapchainSpecification& swapchain_spec);
        ~Swapchain();

        void CleanUp();
        void Invalidate(const SwapchainSpecification& swapchain_spec);
        
        const SwapchainSpecification& GetSpecification() const { return m_Specification; }

        // Surface
        const vk::raii::SwapchainKHR& GetSwapchainKHR() const { return *m_Swapchain; }
        vk::raii::SwapchainKHR& GetSwapchainKHR() { return *m_Swapchain; }

        // Render Pass
        const vk::raii::RenderPass& GetRenderPass() const { return *m_RenderPass; }
        vk::raii::RenderPass& GetRenderPass() { return *m_RenderPass; }

        // Images, Views, and Framebuffers
        uint32_t GetImageCount() const { return m_SwapChainImages.size(); }
        const std::vector<vk::Image>& GetImages() const { return m_SwapChainImages; }
        const std::vector<std::optional<vk::raii::ImageView>>& GetImageViews() const { return m_SwapChainImageViews; }
        const std::vector<std::optional<vk::raii::Framebuffer>>& GetFramebuffers() const { return m_SwapChainFramebuffers; }

    public: // Static Methods

        static SwapChainSupport GetSwapchainSupport(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface);
        static vk::SurfaceFormatKHR ChooseSwapchainFormat(const std::vector<vk::SurfaceFormatKHR>& formats);
        static vk::PresentModeKHR ChooseSwapchainPresentMode(const std::vector<vk::PresentModeKHR>& present_modes);
        static vk::Extent2D ChooseSwapchainExtent(const vk::SurfaceCapabilitiesKHR& capabilities);

        static uint8_t s_MinImageCount;

    private:

        SwapchainSpecification m_Specification;

        std::optional<vk::raii::SwapchainKHR> m_Swapchain{};
        std::optional<vk::raii::RenderPass> m_RenderPass{};

        std::vector<vk::Image> m_SwapChainImages{};
        std::vector<std::optional<vk::raii::ImageView>> m_SwapChainImageViews{};
        std::vector<std::optional<vk::raii::Framebuffer>> m_SwapChainFramebuffers{};
    };

}