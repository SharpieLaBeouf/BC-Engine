#pragma once

// Core Headers
#include "VulkanUtil.h"
#include "Swapchain.h"

#include "Util/Platform.h"

#include "Graphics/Renderer/LightEnvironment.h"

// C++ Standard Library Headers
#include <vector>
#include <thread>
#include <queue>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <optional>

// External Vendor Library Headers
#include <vulkan/vulkan_raii.hpp>
#if defined(BC_PLATFORM_WINDOWS)
#include <vma/vk_mem_alloc.h>
#elif defined(BC_PLATFORM_LINUX)
#include <vk_mem_alloc.h>
#endif

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
    
    #pragma region Initialization and Shutdown

        vk::Result Init(const char* app_name, GLFWwindow* window);
        void Shutdown();

    private:

        vk::Result InitContext();
        vk::Result InitInstance(const char* app_name);
        vk::Result InitDebugMessenger();
        vk::Result InitSurface(GLFWwindow* window);
        vk::Result InitDevice();
        vk::Result InitVMA();
        vk::Result InitSwapChainFramesInFlight();
        vk::Result InitSwapchain(const SwapchainSpecification& swapchain_spec);

    public:

    #pragma endregion

    #pragma region Frame Management

        void BeginFrame();
        void EndFrame();

        void BeginSwapchainRenderPass();
        void EndSwapchainRenderPass();

    #pragma endregion

    #pragma region Getters

        // Instance
        const vk::raii::Instance& GetInstance() const { return *m_Instance; }
        vk::raii::Instance& GetInstance() { return *m_Instance; }
    
        // Surface
        const vk::raii::SurfaceKHR& GetSurface() const { return *m_Surface; }
        vk::raii::SurfaceKHR& GetSurface() { return *m_Surface; }
    
        // Physical Device
        const vk::raii::PhysicalDevice& GetPhysicalDevice() const { return m_PhysicalDevices[m_SelectedDeviceIndex]; }
        vk::raii::PhysicalDevice& GetPhysicalDevice() { return m_PhysicalDevices[m_SelectedDeviceIndex]; }
    
        // Logical Device
        const vk::raii::Device& GetLogicalDevice() const { return *m_LogicalDevice; }
        vk::raii::Device& GetLogicalDevice() { return *m_LogicalDevice; }
    
        // Queues
        uint32_t GetGraphicsQueueFamily() const;
        uint32_t GetComputeQueueFamily() const;
        uint32_t GetPresentQueueFamily() const;
        uint32_t GetTransferQueueFamily() const;

        const vk::raii::Queue& GetGraphicsQueue() const { return *m_GraphicsQueue; }
        vk::raii::Queue& GetGraphicsQueue() { return *m_GraphicsQueue; }

        const vk::raii::Queue& GetComputeQueue() const { return *m_ComputeQueue; }
        vk::raii::Queue& GetComputeQueue() { return *m_ComputeQueue; }

        const vk::raii::Queue& GetPresentQueue() const { return *m_PresentQueue; }
        vk::raii::Queue& GetPresentQueue() { return *m_PresentQueue; }

        const vk::raii::Queue& GetTransferQueue() const { return *m_TransferQueue; }
        vk::raii::Queue& GetTransferQueue() { return *m_TransferQueue; }
        
        // Allocator
        VmaAllocator GetAllocator() const { return m_Allocator; }
        
        // Swapchain
        const Swapchain& GetSwapchain() const { return *m_Swapchain.get(); }

        // Frame's in Flight
        uint32_t GetFrameIndex() const { return m_FrameIndex; }
        uint32_t GetCurrentImageIndex() const { return m_CurrentImageIndex; }
        SwapchainFrameInFlight& GetCurrentFrame() { return m_SwapChainFramesInFlight[m_FrameIndex]; }
        SwapchainFrameInFlight& GetFrameInFlight(uint32_t index) { return m_SwapChainFramesInFlight[index]; }

    #pragma endregion

    #pragma region General Functions

        // Resizing
        void ResizeScreenSpace(uint32_t width, uint32_t height);
        void ResizeSwapchain(const SwapchainSpecification& swapchain_spec);

        // General
        uint32_t FindMemoryType(uint32_t type_filter, vk::MemoryPropertyFlags properties) const;
        
        // Single Use Command Buffers
        vk::raii::CommandPool& GetThreadCommandPool();

        vk::raii::CommandBuffer BeginSingleUseCommandBuffer(vk::raii::CommandPool& pool);
        void EndSingleUseCommandBuffer(vk::raii::Queue& queue, vk::raii::CommandPool& pool, vk::raii::CommandBuffer& cmd);

    #pragma endregion

    private:

        std::optional<vk::raii::Context> m_Context{};
        std::optional<vk::raii::Instance> m_Instance{};
        std::optional<vk::raii::DebugUtilsMessengerEXT> m_DebugMessenger{};

        std::optional<vk::raii::SurfaceKHR> m_Surface{};

        size_t m_SelectedDeviceIndex = -1;
        std::vector<vk::raii::PhysicalDevice> m_PhysicalDevices{};
        std::optional<vk::raii::Device> m_LogicalDevice{};

        struct QueueFamilyIndices 
        {
            std::optional<uint32_t> graphics_family     = 0;
            std::optional<uint32_t> present_family      = 0;
            std::optional<uint32_t> compute_family      = 0;
            std::optional<uint32_t> transfer_family     = 0;
        } m_QueueFamilyIndices{};

        std::optional<vk::raii::Queue> m_GraphicsQueue{};
        std::optional<vk::raii::Queue> m_ComputeQueue{};
        std::optional<vk::raii::Queue> m_PresentQueue{};
        std::optional<vk::raii::Queue> m_TransferQueue{};

        VmaAllocator m_Allocator = VK_NULL_HANDLE;
        
        uint32_t m_CurrentImageIndex;
        std::unique_ptr<Swapchain> m_Swapchain = nullptr;

        struct SwapchainFrameInFlight
        {            
            // Does not need to be threadlocal as the Swapchain frame in flight is only used by the Render thread.

            std::optional<vk::raii::CommandPool>        command_pool;
            std::optional<vk::raii::CommandBuffer>      command_buffer;
            std::optional<vk::raii::DescriptorPool>     descriptor_pool;

            uint32_t                                    image_index;
            std::optional<vk::raii::Semaphore>          image_available_semaphore;
            std::optional<vk::raii::Semaphore>          render_finished_semaphore;
            std::optional<vk::raii::Fence>              in_flight_fence;
        };

        uint32_t m_FrameIndex = 0;
        std::vector<SwapchainFrameInFlight> m_SwapChainFramesInFlight;

        std::mutex m_UploadPoolMutex;
        std::unordered_map<std::thread::id, vk::raii::CommandPool> m_ThreadUploadPools;
    };

}