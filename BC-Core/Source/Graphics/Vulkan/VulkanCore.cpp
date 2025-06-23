#include "BC_PCH.h"
#include "VulkanCore.h"

#include "Core/Application.h"

#include <GLFW/glfw3.h>

namespace BC
{

    VkResult VulkanCore::Init(const char *app_name, GLFWwindow *window)
    {
        BC_CATCH_BEGIN();

        VkResult result;

        // 1. Instance
        result = CreateInstance(app_name);
        BC_THROW(result == VK_SUCCESS, "VulkanCore::Init: Failed to Create Vulkan Instance.");
        BC_CORE_TRACE("VulkanCore::Init: Created Instance.");

        // 2. Debug Messenger
        result = CreateDebugMessenger();
        BC_THROW(result == VK_SUCCESS, "VulkanCore::Init: Failed to Create Debug Messenger.");
        BC_CORE_TRACE("VulkanCore::Init: Debug Messenger Created.");

        // 3. Surface
        result = CreateSurface(window);
        BC_THROW(result == VK_SUCCESS, "VulkanCore::Init: Failed to Create Surface.");
        BC_CORE_TRACE("VulkanCore::Init: Created Surface.");

        // 4. Device
        result = CreateDevice();
        BC_THROW(result == VK_SUCCESS, "VulkanCore::Init: Failed to Create Device.");
        BC_CORE_TRACE("VulkanCore::Init: Created Device.");

        // 5. Globals
        result = CreateGlobals();
        BC_THROW(result == VK_SUCCESS, "VulkanCore::Init: Failed to Create Globals.");
        BC_CORE_TRACE("VulkanCore::Init: Created Globals.");

        // 6. Swapchain
        auto capabilities = Swapchain::GetSwapchainSupport(GetPhysicalDevice(), GetSurface());
        SwapchainSpecification swapchain_spec = 
        {
            .ImageCount = static_cast<uint8_t>(Swapchain::s_MinImageCount + 1),
            .ImageFormat = Swapchain::ChooseSwapchainFormat(capabilities.Formats),
            .PresentMode = Swapchain::ChooseSwapchainPresentMode(capabilities.PresentModes),
            .Extent = Swapchain::ChooseSwapchainExtent(capabilities.Capabilities)
        };
        result = CreateSwapchain(swapchain_spec);
        BC_THROW(result == VK_SUCCESS, "VulkanCore::Init: Failed to Create Swapchain.");
        BC_CORE_TRACE("VulkanCore::Init: Created Swapchain.");

        // 7. SwapChainFramesInFlight
        result = CreateSwapChainFramesInFlight();
        BC_THROW(result == VK_SUCCESS, "VulkanCore::Init: Failed to Create Swap Chain Frames in Flight.");
        BC_CORE_TRACE("VulkanCore::Init: Created Swap Chain Frames in Flight.");

        BC_THROW(m_Instance,       "VulkanCore::Init: Instance Not Valid");
        if (Util::s_EnableValidationLayer)
            BC_THROW(m_DebugMessenger, "VulkanCore::Init: Debug Messenger Not Valid");
        BC_THROW(m_Surface,        "VulkanCore::Init: Surface Not Valid");
        BC_THROW(m_LogicalDevice,  "VulkanCore::Init: Logical Device Not Valid");

        BC_THROW(m_Swapchain,      "VulkanCore::Init: Swapchain Not Valid");
        BC_THROW(m_GraphicsQueue,  "VulkanCore::Init: Graphics Queue Not Valid");
        BC_THROW(m_ComputeQueue,   "VulkanCore::Init: Compute Queue Not Valid");
        BC_THROW(m_PresentQueue,   "VulkanCore::Init: Present Queue Not Valid");

        BC_CATCH_END_RETURN(VK_ERROR_UNKNOWN);

        return VK_SUCCESS;
    }

    void VulkanCore::Shutdown()
    {

        for (auto& frame : m_SwapChainFramesInFlight)
        {
            if (frame.image_available_semaphore)
                vkDestroySemaphore(m_LogicalDevice, frame.image_available_semaphore, nullptr);
            if (frame.render_finished_semaphore)
                vkDestroySemaphore(m_LogicalDevice, frame.render_finished_semaphore, nullptr);
            if (frame.in_flight_fence)
                vkDestroyFence(m_LogicalDevice, frame.in_flight_fence, nullptr);
            if (frame.command_pool)
                vkDestroyCommandPool(m_LogicalDevice, frame.command_pool, nullptr);
        }
        m_SwapChainFramesInFlight.clear();

        m_Swapchain.reset();
        m_Swapchain = nullptr;

        for (auto& [tid, pool] : m_ThreadUploadPools)
            vkDestroyCommandPool(m_LogicalDevice, pool, nullptr);
        m_ThreadUploadPools.clear();

        if (m_DefaultSampler)
            vkDestroySampler(m_LogicalDevice, m_DefaultSampler, nullptr);
        if (m_StaticDescriptorPool)
            vkDestroyDescriptorPool(m_LogicalDevice, m_StaticDescriptorPool, nullptr);
        if (m_ImageSamplerSetLayout)
            vkDestroyDescriptorSetLayout(m_LogicalDevice, m_ImageSamplerSetLayout, nullptr);

        if (m_LogicalDevice)
            vkDestroyDevice(m_LogicalDevice, VK_NULL_HANDLE);

        if (m_Surface)
            vkDestroySurfaceKHR(m_Instance, m_Surface, VK_NULL_HANDLE);

        if (auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugUtilsMessengerEXT"); func != nullptr) 
        { 
            func(m_Instance, m_DebugMessenger, VK_NULL_HANDLE); 
        }

        if (m_Instance)
            vkDestroyInstance(m_Instance, VK_NULL_HANDLE);
    }

    void VulkanCore::BeginFrame()
    {
        BC_THROW(!m_SwapChainFramesInFlight.empty(), "VulkanCore::BeginFrame: Frame Sync in Flight Vector Empty.");

        auto& frame = m_SwapChainFramesInFlight[m_FrameIndex];

        VkResult result = vkWaitForFences(m_LogicalDevice, 1, &frame.in_flight_fence, VK_TRUE, UINT64_MAX);
        BC_THROW(result == VK_SUCCESS, "VulkanCore::BeginFrame: Could Not Wait For Fences.");

        result = vkResetFences(m_LogicalDevice, 1, &frame.in_flight_fence);
        BC_THROW(result == VK_SUCCESS, "VulkanCore::BeginFrame: Could Not Reset Fences.");

        result = vkResetCommandPool(m_LogicalDevice, frame.command_pool, 0);
        BC_THROW(result == VK_SUCCESS, "VulkanCore::BeginFrame: Could Not Reset Command Pool.");

        result = vkAcquireNextImageKHR(m_LogicalDevice, m_Swapchain->GetSwapchain(), UINT64_MAX, frame.image_available_semaphore, VK_NULL_HANDLE, &m_CurrentImageIndex);
        BC_THROW(result == VK_SUCCESS, "VulkanCore::BeginFrame: Could Not Acquire Next Image.");
        
        frame.image_index = m_CurrentImageIndex;

        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = 0;
        begin_info.pInheritanceInfo = nullptr;

        result = vkBeginCommandBuffer(frame.command_buffer, &begin_info);
        BC_THROW(result == VK_SUCCESS, "VulkanCore::BeginFrame: Could Not Begin Command Buffer.");

    }
    
    void VulkanCore::EndFrame()
    {
        auto& frame = m_SwapChainFramesInFlight[m_FrameIndex];

        VkResult result = vkEndCommandBuffer(frame.command_buffer);
        BC_THROW(result == VK_SUCCESS, "VulkanCore::EndFrame: Could Not End Command Buffer.");

        VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &frame.image_available_semaphore;
        submit_info.pWaitDstStageMask = &wait_stage;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &frame.command_buffer;
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &frame.render_finished_semaphore;

        result = vkQueueSubmit(m_GraphicsQueue, 1, &submit_info, frame.in_flight_fence);
        BC_THROW(result == VK_SUCCESS, "VulkanCore::EndFrame: Could Not Submit Command Buffer.");

        VkSwapchainKHR swapchain = m_Swapchain->GetSwapchain();

        VkPresentInfoKHR present_info = {};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &frame.render_finished_semaphore;
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &swapchain;
        present_info.pImageIndices = &m_CurrentImageIndex;

        result = vkQueuePresentKHR(m_PresentQueue, &present_info);
        BC_THROW(result == VK_SUCCESS, "VulkanCore::EndFrame: Could Not Present.");

        m_FrameIndex = (m_FrameIndex + 1) % m_SwapChainFramesInFlight.size();
    }

    void VulkanCore::BeginSwapchainRenderPass()
    {
        auto& frame = m_SwapChainFramesInFlight[m_FrameIndex];

        VkRenderPassBeginInfo render_pass_info{};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.renderPass = m_Swapchain->GetRenderPass();
        render_pass_info.framebuffer = m_Swapchain->GetFramebuffers()[frame.image_index];
        render_pass_info.renderArea.offset = { 0, 0 };
        render_pass_info.renderArea.extent = m_Swapchain->GetSpecification().Extent;

        VkClearValue clear_color{};
        clear_color.color = { {0.0f, 0.0f, 0.0f, 1.0f} };
        render_pass_info.clearValueCount = 1;
        render_pass_info.pClearValues = &clear_color;

        vkCmdBeginRenderPass(frame.command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
    }

    void VulkanCore::EndSwapchainRenderPass()
    {
        vkCmdEndRenderPass(m_SwapChainFramesInFlight[m_FrameIndex].command_buffer);
    }

    VkInstance VulkanCore::GetInstance() const 
    { 
        return m_Instance; 
    }

    VkSurfaceKHR VulkanCore::GetSurface() const
    {
        return m_Surface;
    }

    VkPhysicalDevice VulkanCore::GetPhysicalDevice() const
    {
        if (m_SelectedDeviceIndex < 0 || m_SelectedDeviceIndex >= m_PhysicalDevices.size())
            return VK_NULL_HANDLE;

        return m_PhysicalDevices[m_SelectedDeviceIndex]; 
    }

    const VkDevice VulkanCore::GetLogicalDevice() const
    {
        return m_LogicalDevice;
    }

    uint32_t VulkanCore::GetGraphicsQueueFamily() const
    { 
        if (!m_QueueFamilyIndices.graphics_family.has_value())
            return -1;

        return m_QueueFamilyIndices.graphics_family.value();
    }

    uint32_t VulkanCore::GetComputeQueueFamily() const
    { 
        if (!m_QueueFamilyIndices.compute_family.has_value())
            return -1;

        return m_QueueFamilyIndices.compute_family.value();
    }

    uint32_t VulkanCore::GetPresentQueueFamily() const
    { 
        if (!m_QueueFamilyIndices.present_family.has_value())
            return -1;

        return m_QueueFamilyIndices.present_family.value();
    }

    VkQueue VulkanCore::GetGraphicsQueue() const
    {
        return m_GraphicsQueue;
    }

    VkQueue VulkanCore::GetComputeQueue() const
    {
        return m_ComputeQueue;
    }

    VkQueue VulkanCore::GetPresentQueue() const
    {
        return m_PresentQueue;
    }

    void VulkanCore::ResizeScreenSpace(uint32_t width, uint32_t height)
    {
        // TODO: Do we need to do anything here?
    }

    void VulkanCore::ResizeSwapchain(const SwapchainSpecification& swapchain_spec)
    {
        BC_CATCH_BEGIN();

        vkDeviceWaitIdle(m_LogicalDevice);

        m_Swapchain.reset();
        m_Swapchain = std::make_unique<Swapchain>(swapchain_spec);

        BC_CATCH_END_FUNC([]() { Application::Get()->Close(); });
    }

    uint32_t VulkanCore::FindMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties) const
    {
        VkPhysicalDevice physical_device = GetPhysicalDevice();

        VkPhysicalDeviceMemoryProperties mem_properties;
        vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);

        for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++)
        {
            if ((type_filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        BC_THROW(false, "VulkanCore::FindMemoryType: Failed to find suitable memory type!");
    }

    VkCommandPool VulkanCore::GetThreadUploadCommandPool()
    {
        std::lock_guard<std::mutex> lock(m_UploadPoolMutex);

        auto thread_id = std::this_thread::get_id();
        auto it = m_ThreadUploadPools.find(thread_id);
        if (it != m_ThreadUploadPools.end())
            return it->second;

        VkCommandPoolCreateInfo pool_info{};
        pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_info.queueFamilyIndex = m_QueueFamilyIndices.graphics_family.value();
        pool_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        VkCommandPool command_pool;
        vkCreateCommandPool(m_LogicalDevice, &pool_info, nullptr, &command_pool);
        m_ThreadUploadPools[thread_id] = command_pool;

        return command_pool;
    }

    VkCommandBuffer VulkanCore::BeginSingleUseCommandBuffer(VkCommandPool pool)
    {
        VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandPool = pool;
        alloc_info.commandBufferCount = 1;

        VkCommandBuffer cmd;
        vkAllocateCommandBuffers(m_LogicalDevice, &alloc_info, &cmd);

        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(cmd, &begin_info);
        return cmd;
    }

    void VulkanCore::EndSingleUseCommandBuffer(VkQueue queue, VkCommandPool pool, VkCommandBuffer cmd)
    {
        vkEndCommandBuffer(cmd);

        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &cmd;

        vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
        vkQueueWaitIdle(queue);

        vkFreeCommandBuffers(m_LogicalDevice, pool, 1, &cmd);
    }

    VkResult VulkanCore::CreateInstance(const char *app_name)
    {
        if (Util::s_EnableValidationLayer && !Util::CheckValidationLayerSupport()) 
        {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        VkApplicationInfo app_info = 
        {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = VK_NULL_HANDLE,
            .pApplicationName = app_name,
            .applicationVersion = 1,
            .pEngineName = VK_NULL_HANDLE,
            .engineVersion = 1,
            .apiVersion = VK_API_VERSION_1_3,
        };

        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;

        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions;

        for (uint32_t i = 0; i < glfwExtensionCount; ++i)
        {
            extensions.push_back(glfwExtensions[i]);
        }

        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        VkDebugUtilsMessengerCreateInfoEXT debug_create_info = 
        {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .pNext = VK_NULL_HANDLE,
            .flags = 0,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = Util::VK_DebugCallback,
            .pUserData = VK_NULL_HANDLE

        };

        std::vector<const char*> validation_layers = { "VK_LAYER_KHRONOS_validation" };

        VkInstanceCreateInfo instance_create = 
        {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = (Util::s_EnableValidationLayer) ? (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info : VK_NULL_HANDLE,
            .flags = 0,
            .pApplicationInfo = &app_info,
            .enabledLayerCount = (Util::s_EnableValidationLayer) ? static_cast<uint32_t>(validation_layers.size()) : 0,
            .ppEnabledLayerNames = (Util::s_EnableValidationLayer) ? validation_layers.data() : VK_NULL_HANDLE,
            .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
            .ppEnabledExtensionNames = extensions.data()
        };

        return vkCreateInstance(&instance_create, VK_NULL_HANDLE, &m_Instance);
    }

    VkResult VulkanCore::CreateDebugMessenger()
    {
        if (!Util::s_EnableValidationLayer)
        {
            return VK_SUCCESS;
        }

        VkDebugUtilsMessengerCreateInfoEXT debug_create_info = 
        {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .pNext = VK_NULL_HANDLE,
            .flags = 0,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = Util::VK_DebugCallback,
            .pUserData = VK_NULL_HANDLE

        };

        if (auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(m_Instance, "vkCreateDebugUtilsMessengerEXT");func != nullptr) 
        {
            func(m_Instance, &debug_create_info, VK_NULL_HANDLE, &m_DebugMessenger);
        } 
        else 
        {
            return VK_ERROR_UNKNOWN;
        }
        
        return VK_SUCCESS;
    }

    VkResult VulkanCore::CreateSurface(GLFWwindow *window)
    {
        return glfwCreateWindowSurface(m_Instance, window, VK_NULL_HANDLE, &m_Surface);
    }

    VkResult VulkanCore::CreateDevice()
    {
        uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(m_Instance, &device_count, VK_NULL_HANDLE);
        BC_THROW(device_count != 0, "VulkanCore::CreateDevice - No Devices Found.");

        m_PhysicalDevices.resize(device_count);
        vkEnumeratePhysicalDevices(m_Instance, &device_count, m_PhysicalDevices.data());

        // Pick Device
        {
            struct Candidate
            {
                VkPhysicalDevice device;
                int score;
            };

            std::vector<Candidate> candidates;

            for (auto device : m_PhysicalDevices)
            {
                VkPhysicalDeviceProperties properties;
                VkPhysicalDeviceFeatures features;
                vkGetPhysicalDeviceProperties(device, &properties);
                vkGetPhysicalDeviceFeatures(device, &features);

                int score = 0;

                // Prefer discrete GPUs
                if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
                    score += 1000;

                // Check for required queue families (e.g., graphics)
                uint32_t queue_family_count = 0;
                vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
                std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
                vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

                bool has_graphics_queue = false;
                for (uint32_t i = 0; i < queue_family_count; ++i)
                {
                    if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                    {
                        has_graphics_queue = true;
                        break;
                    }
                }

                if (!has_graphics_queue)
                    continue;

                // Example: Check for required extensions (like swapchain)
                uint32_t extension_count = 0;
                vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);
                std::vector<VkExtensionProperties> available_extensions(extension_count);
                vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions.data());

                std::vector<const char*> required_extensions = {
                    VK_KHR_SWAPCHAIN_EXTENSION_NAME
                };

                bool extensions_supported = true;
                for (const char* req_ext : required_extensions)
                {
                    bool found = false;
                    for (const auto& ext : available_extensions)
                    {
                        if (strcmp(req_ext, ext.extensionName) == 0)
                        {
                            found = true;
                            break;
                        }
                    }
                    if (!found)
                    {
                        extensions_supported = false;
                        break;
                    }
                }

                if (!extensions_supported)
                    continue;

                // Add more feature checks here as needed...

                candidates.push_back({ device, score });
            }

            if (candidates.empty())
            {
                std::cerr << "Error: No suitable physical device found!\n";
                return VK_ERROR_UNKNOWN;
            }

            // Pick the best device
            std::sort(candidates.begin(), candidates.end(),
                    [](const Candidate& a, const Candidate& b) { return a.score > b.score; });

            m_SelectedDeviceIndex = -1;
            for (auto i = 0; i < m_PhysicalDevices.size(); ++i)
            {
                if (m_PhysicalDevices[i] == candidates[0].device)
                {
                    m_SelectedDeviceIndex = i;
                    break;
                }
            }
            BC_ASSERT(m_SelectedDeviceIndex != -1, "Vulkan::CreateDevice - Invalid Selected Index.");

            uint32_t queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevices[m_SelectedDeviceIndex], &queueFamilyCount, nullptr);

            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevices[m_SelectedDeviceIndex], &queueFamilyCount, queueFamilies.data());
            
            int i = 0;
            for (const auto& queueFamily : queueFamilies) 
            {
                if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) 
                {
                    m_QueueFamilyIndices.graphics_family = i;
                }
                
                if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) 
                {
                    m_QueueFamilyIndices.compute_family = i;
                }

                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(m_PhysicalDevices[m_SelectedDeviceIndex], i, m_Surface, &presentSupport);
                if (presentSupport) {
                    m_QueueFamilyIndices.present_family = i;
                }

                i++;
            }

            if (!m_QueueFamilyIndices.graphics_family.has_value() || !m_QueueFamilyIndices.compute_family.has_value() || !m_QueueFamilyIndices.present_family.has_value())
            {
                Shutdown();
                std::cerr << "Error: Device Queues Not Contain All Functionality!\n";
                return VK_ERROR_UNKNOWN;
            }

            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(m_PhysicalDevices[m_SelectedDeviceIndex], &properties);
            BC_CORE_TRACE("Selected Device: {}", properties.deviceName);
        }
        
        float queuePriority = 1.0f;
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> unique_queue_families = { m_QueueFamilyIndices.graphics_family.value(), m_QueueFamilyIndices.present_family.value(), m_QueueFamilyIndices.compute_family.value() };

        for (uint32_t queueFamily : unique_queue_families) {
            
            VkDeviceQueueCreateInfo queue_create = 
            {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .pNext = VK_NULL_HANDLE,
                .flags = 0,
                .queueFamilyIndex = queueFamily,
                .queueCount = 1,
                .pQueuePriorities = &queuePriority
            };
            queueCreateInfos.push_back(queue_create);
        }

        VkPhysicalDeviceFeatures device_features = {};
        vkGetPhysicalDeviceFeatures(GetPhysicalDevice(), &device_features);
        const std::vector<const char*> device_extensions = 
        { 
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

        VkDeviceCreateInfo device_create = 
        {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = VK_NULL_HANDLE,
            .flags = 0,
            
            .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
            .pQueueCreateInfos = queueCreateInfos.data(),

            .enabledLayerCount = 0,
            .ppEnabledLayerNames = VK_NULL_HANDLE,

            .enabledExtensionCount = static_cast<uint32_t>(device_extensions.size()),
            .ppEnabledExtensionNames = device_extensions.data(),

            .pEnabledFeatures = &device_features
        };

        if (auto result = vkCreateDevice(m_PhysicalDevices[m_SelectedDeviceIndex], &device_create, VK_NULL_HANDLE, &m_LogicalDevice); result != VK_SUCCESS)
        {
            std::cerr << "Error: Could Not Create Logical Device!\n";
            return result;
        }

        vkGetDeviceQueue(m_LogicalDevice, m_QueueFamilyIndices.graphics_family.value(), 0, &m_GraphicsQueue);
        vkGetDeviceQueue(m_LogicalDevice, m_QueueFamilyIndices.present_family.value(), 0, &m_PresentQueue);
        vkGetDeviceQueue(m_LogicalDevice, m_QueueFamilyIndices.compute_family.value(), 0, &m_ComputeQueue);

        return VK_SUCCESS;
    }

    VkResult VulkanCore::CreateGlobals()
    {
        VkSamplerCreateInfo sampler_info{};
        sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler_info.magFilter = VK_FILTER_LINEAR;
        sampler_info.minFilter = VK_FILTER_LINEAR;
        sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_info.anisotropyEnable = VK_FALSE;
        sampler_info.maxAnisotropy = 1.0f;

        VkResult result = vkCreateSampler(m_LogicalDevice, &sampler_info, nullptr, &m_DefaultSampler);
        if (result != VK_SUCCESS)
            return result;

        // Descriptor Pool
        VkDescriptorPoolSize pool_size{};
        pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        pool_size.descriptorCount = 1024;

        VkDescriptorPoolCreateInfo pool_info{};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.poolSizeCount = 1;
        pool_info.pPoolSizes = &pool_size;
        pool_info.maxSets = 1024;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

        result = vkCreateDescriptorPool(m_LogicalDevice, &pool_info, nullptr, &m_StaticDescriptorPool);
        if (result != VK_SUCCESS)
            return result;

        // Descriptor Set Layout
        VkDescriptorSetLayoutBinding binding{};
        binding.binding = 0;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding.descriptorCount = 1;
        binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo layout_info{};
        layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout_info.bindingCount = 1;
        layout_info.pBindings = &binding;

        result = vkCreateDescriptorSetLayout(m_LogicalDevice, &layout_info, nullptr, &m_ImageSamplerSetLayout);

        return result;
    }

    VkResult VulkanCore::CreateSwapChainFramesInFlight()
    {
        BC_ASSERT(m_Swapchain, "VulkanCore::CreateSyncObjects: SwapChain Needs to be Valid.");

        m_SwapChainFramesInFlight.resize(m_Swapchain->GetImageCount());

        VkSemaphoreCreateInfo semaphore_info{};
        semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fence_info{};
        fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Start signaled so first frame doesn't wait forever

        VkResult result;
        for (auto& frame : m_SwapChainFramesInFlight)
        {
            result = vkCreateSemaphore(m_LogicalDevice, &semaphore_info, nullptr, &frame.image_available_semaphore);
            if (result != VK_SUCCESS) 
                return result;

            result = vkCreateSemaphore(m_LogicalDevice, &semaphore_info, nullptr, &frame.render_finished_semaphore);
            if (result != VK_SUCCESS) 
                return result;

            result = vkCreateFence(m_LogicalDevice, &fence_info, nullptr, &frame.in_flight_fence);
            if (result != VK_SUCCESS) 
                return result;

            // Create Command Pool
            VkCommandPoolCreateInfo pool_info{};
            pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            pool_info.queueFamilyIndex = m_QueueFamilyIndices.graphics_family.value();

            result = vkCreateCommandPool(m_LogicalDevice, &pool_info, nullptr, &frame.command_pool);
            if (result != VK_SUCCESS)
                return result;

            // Allocate command buffer
            VkCommandBufferAllocateInfo alloc_info{};
            alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            alloc_info.commandPool = frame.command_pool;
            alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            alloc_info.commandBufferCount = 1;

            result = vkAllocateCommandBuffers(m_LogicalDevice, &alloc_info, &frame.command_buffer);
            if (result != VK_SUCCESS)
                return result;
        }

        return VK_SUCCESS;
    }

    VkResult VulkanCore::CreateSwapchain(const SwapchainSpecification& swapchain_spec)
    {
        BC_CATCH_BEGIN();

        m_Swapchain.reset();
        m_Swapchain = std::make_unique<Swapchain>(swapchain_spec);

        BC_CATCH_END_RETURN(VK_ERROR_UNKNOWN);
        
        return VK_SUCCESS;
    }
    
}