#pragma once

// Core Headers
#include "VulkanUtil.h"
#include "Swapchain.h"
#include "RenderCommand.h"

#include "Graphics/Renderer/LightEnvironment.h"

// C++ Standard Library Headers
#include <vector>
#include <thread>
#include <queue>
#include <unordered_map>
#include <mutex>
#include <thread>

// External Vendor Library Headers
#include <vulkan/vulkan.h>

typedef struct GLFWwindow GLFWwindow;

namespace BC
{

    class VulkanCore
    {
        struct SwapchainFrameInFlight;

    public:

        VulkanCore() = default;
        ~VulkanCore() { Shutdown(); }

        VulkanCore(const VulkanCore& other) = delete;
        VulkanCore(VulkanCore&& other) = delete;

        VulkanCore& operator=(const VulkanCore& other) = delete;
        VulkanCore& operator=(VulkanCore&& other) = delete;

        VkResult Init(const char* app_name, GLFWwindow* window);
        void Shutdown();

        void BeginFrame();
        void EndFrame();

        void BeginSwapchainRenderPass();
        void EndSwapchainRenderPass();

        VkInstance GetInstance() const;
        VkSurfaceKHR GetSurface() const;

        VkPhysicalDevice GetPhysicalDevice() const;
        const VkDevice GetLogicalDevice() const;

        const Swapchain& GetSwapchain() const { return *m_Swapchain.get(); }

        uint32_t GetGraphicsQueueFamily() const;
        uint32_t GetComputeQueueFamily() const;
        uint32_t GetPresentQueueFamily() const;
        VkQueue GetGraphicsQueue() const;
        VkQueue GetComputeQueue() const;
        VkQueue GetPresentQueue() const;

        void ResizeScreenSpace(uint32_t width, uint32_t height);
        void ResizeSwapchain(const SwapchainSpecification& swapchain_spec);

        uint32_t GetCurrentImageIndex() const { return m_CurrentImageIndex; }
        uint32_t GetFrameIndex() const { return m_FrameIndex; }
        SwapchainFrameInFlight& GetCurrentFrame() { return m_SwapChainFramesInFlight[m_FrameIndex]; }

        uint32_t FindMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties) const;

        VkSampler GetDefaultSampler() const { return m_DefaultSampler; }
        VkDescriptorPool GetStaticDescriptorPool() const { return m_StaticDescriptorPool; }
        VkDescriptorSetLayout GetImageSamplerSetLayout() const { return m_ImageSamplerSetLayout; }
        
        VkCommandPool GetThreadUploadCommandPool();
        VkCommandBuffer BeginSingleUseCommandBuffer(VkCommandPool pool);
        void EndSingleUseCommandBuffer(VkQueue queue, VkCommandPool pool, VkCommandBuffer cmd);

    private:

        VkResult CreateInstance(const char* app_name);
        VkResult CreateDebugMessenger();
        VkResult CreateSurface(GLFWwindow* window);
        VkResult CreateDevice();
        VkResult CreateGlobals();
        VkResult CreateSwapChainFramesInFlight();
        VkResult CreateSwapchain(const SwapchainSpecification& swapchain_spec);

    private:

        VkInstance m_Instance = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
        
        VkSurfaceKHR m_Surface = VK_NULL_HANDLE;

        size_t m_SelectedDeviceIndex = -1;
        VkDevice m_LogicalDevice = VK_NULL_HANDLE;
        std::vector<VkPhysicalDevice> m_PhysicalDevices;

        struct QueueFamilyIndices 
        {
            std::optional<uint32_t> graphics_family;
            std::optional<uint32_t> compute_family;
            std::optional<uint32_t> present_family;
        } m_QueueFamilyIndices;

        VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
        VkQueue m_ComputeQueue = VK_NULL_HANDLE;
        VkQueue m_PresentQueue = VK_NULL_HANDLE;
        
        std::unique_ptr<Swapchain> m_Swapchain = nullptr;
        uint32_t m_CurrentImageIndex;

        uint32_t m_FrameIndex = 0;

        struct SwapchainFrameInFlight
        {
            VkCommandPool   command_pool;
            VkCommandBuffer command_buffer;

            uint32_t    image_index;
            VkSemaphore image_available_semaphore;
            VkSemaphore render_finished_semaphore;
            VkFence     in_flight_fence;

            void* user_data;
        };
        std::vector<SwapchainFrameInFlight> m_SwapChainFramesInFlight;

        VkSampler m_DefaultSampler = VK_NULL_HANDLE;
        VkDescriptorPool m_StaticDescriptorPool = VK_NULL_HANDLE;
        VkDescriptorSetLayout m_ImageSamplerSetLayout = VK_NULL_HANDLE;

        std::mutex m_UploadPoolMutex;
        std::unordered_map<std::thread::id, VkCommandPool> m_ThreadUploadPools;
    };

}