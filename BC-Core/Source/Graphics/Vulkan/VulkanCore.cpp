#include "BC_PCH.h"
#include "VulkanCore.h"

#include "Core/Application.h"

#include <GLFW/glfw3.h>

namespace BC
{

#pragma region Init and Shutdown

    vk::Result VulkanCore::Init(const char *app_name, GLFWwindow *window)
    {
        BC_CATCH_BEGIN();

        // Check if validation layer is supported
        if (Util::s_EnableValidationLayer && !Util::IsValidationLayerSupported())
        {
            BC_CORE_WARN("VulkanCore::Init: Vulkan Validation Layer is not supported on this system.");
            Util::s_EnableValidationLayer = false;
        }

        vk::Result result;

        // 1. Create Context
        result = InitContext();
        BC_THROW(result == vk::Result::eSuccess, "VulkanCore::Init: Failed to Create Vulkan Context.");
        BC_CORE_TRACE("VulkanCore::Init: Created Vulkan Context.");

        // 2. Create Instance
        result = InitInstance(app_name);
        BC_THROW(result == vk::Result::eSuccess, "VulkanCore::Init: Failed to Create Vulkan Instance.");
        BC_CORE_TRACE("VulkanCore::Init: Created Instance.");

        // 3. Create Debug Messenger
        result = InitDebugMessenger();
        BC_THROW(result == vk::Result::eSuccess, "VulkanCore::Init: Failed to Create Debug Messenger.");
        BC_CORE_TRACE("VulkanCore::Init: Debug Messenger Created.");

        // 4. Create Surface
        result = InitSurface(window);
        BC_THROW(result == vk::Result::eSuccess, "VulkanCore::Init: Failed to Create Surface.");
        BC_CORE_TRACE("VulkanCore::Init: Created Surface.");

        // 5. Create Device
        result = InitDevice();
        BC_THROW(result == vk::Result::eSuccess, "VulkanCore::Init: Failed to Create Device.");
        BC_CORE_TRACE("VulkanCore::Init: Created Device.");

        // 6. Create VMA Allocator
        result = InitVMA();
        BC_THROW(result == vk::Result::eSuccess, "VulkanCore::Init: Failed to Create VMA Allocator.");
        BC_CORE_TRACE("VulkanCore::Init: Created VMA Allocator.");

        // 7. Create Swapchain
        auto capabilities = Swapchain::GetSwapchainSupport(GetPhysicalDevice(), GetSurface());
        Swapchain::s_MinImageCount = std::max<uint32_t>(capabilities.capabilities.minImageCount, 2);
        BC_THROW(Swapchain::s_MinImageCount >= 2, "VulkanCore::Init: Min Image Count Must Be Atleast 2.");

        SwapchainSpecification swapchain_spec =
        {
            .image_count = static_cast<uint8_t>(Swapchain::s_MinImageCount + 1),
            .image_format = Swapchain::ChooseSwapchainFormat(capabilities.formats),
            .present_mode = Swapchain::ChooseSwapchainPresentMode(capabilities.present_modes),
            .extent = Swapchain::ChooseSwapchainExtent(capabilities.capabilities)
        };

        result = InitSwapchain(swapchain_spec);
        BC_THROW(result == vk::Result::eSuccess, "VulkanCore::Init: Failed to Create Swapchain.");

        // 8. SwapChainFramesInFlight
        result = InitSwapChainFramesInFlight();
        BC_THROW(result == vk::Result::eSuccess, "VulkanCore::Init: Failed to Create Swap Chain Frames in Flight.");
        BC_CORE_TRACE("VulkanCore::Init: Created Swap Chain Frames in Flight.");

        // 9. Validate All Initializations
        BC_THROW(m_Context,                                         "VulkanCore::Init: Context Not Valid");
        BC_THROW(m_Instance,                                        "VulkanCore::Init: Instance Not Valid");
        
        if (Util::s_EnableValidationLayer)
            BC_THROW(m_DebugMessenger,                              "VulkanCore::Init: Debug Messenger Not Valid");

        BC_THROW(m_Surface,                                         "VulkanCore::Init: Surface Not Valid");

        BC_THROW(m_PhysicalDevices.size() > 0,                      "VulkanCore::Init: Physical Devices Not Valid");
        BC_THROW(m_SelectedDeviceIndex < m_PhysicalDevices.size(),  "VulkanCore::Init: Selected Device Index Out Of Range");
        BC_THROW(m_LogicalDevice,                                   "VulkanCore::Init: Logical Device Not Valid");

        BC_THROW(m_Allocator,                                       "VulkanCore::Init: VMA Allocator Not Valid");

        BC_THROW(m_Swapchain,                                       "VulkanCore::Init: Swapchain Not Valid");
        BC_THROW(*m_Swapchain->GetSwapchainKHR(),                   "VulkanCore::Init: Swapchain Handle Not Valid");
        BC_THROW(*m_Swapchain->GetRenderPass(),                     "VulkanCore::Init: Swapchain Render Pass Not Valid");
        BC_THROW(!m_SwapChainFramesInFlight.empty(),                "VulkanCore::Init: Swap Chain Frames in Flight Vector Empty");

        BC_THROW(m_GraphicsQueue,                                   "VulkanCore::Init: Graphics Queue Not Valid");
        BC_THROW(m_ComputeQueue,                                    "VulkanCore::Init: Compute Queue Not Valid");
        BC_THROW(m_PresentQueue,                                    "VulkanCore::Init: Present Queue Not Valid");
        BC_THROW(m_TransferQueue,                                   "VulkanCore::Init: Transfer Queue Not Valid");

        BC_CATCH_END_RETURN(vk::Result::eErrorInitializationFailed);

        return vk::Result::eSuccess;
    }

    void VulkanCore::Shutdown()
    {
        // Wait for device to finish operations
        if (m_LogicalDevice)
            m_LogicalDevice->waitIdle();
    
        // Clear swapchain frames in flight
        m_SwapChainFramesInFlight.clear();
    
        // Clear thread upload pools
        m_ThreadUploadPools.clear();
    
        // Reset swapchain
        m_Swapchain.reset();
    
        // Destroy VMA allocator
        if (m_Allocator)
        {
            vmaDestroyAllocator(m_Allocator);
            m_Allocator = VK_NULL_HANDLE;
        }
    
        // Reset logical device
        m_LogicalDevice.reset();
    
        // Reset surface
        m_Surface.reset();
    
        // Reset debug messenger
        m_DebugMessenger.reset();
    
        // Reset instance
        m_Instance.reset();
    
        // Reset context
        m_Context.reset();
    }

    vk::Result VulkanCore::InitContext()
    {
        if (m_Context.has_value())
        {
            BC_CORE_WARN("VulkanCore::InitContext: Context Already Initialized.");
            return vk::Result::eSuccess;
        }

        try
        {
            m_Context.emplace();
        }
        catch (const vk::SystemError& e)
        {
            auto result = static_cast<vk::Result>(e.code().value());
            BC_CORE_ERROR("VulkanCore::InitContext: Failed to Create Vulkan Context.");
            return result;
        }

        return vk::Result::eSuccess;
    }

    vk::Result VulkanCore::InitInstance(const char *app_name)
    {
        vk::ApplicationInfo app_info = {};
        app_info.pApplicationName = app_name;
        app_info.applicationVersion = 1;
        app_info.engineVersion = 1;
        app_info.apiVersion = VK_API_VERSION_1_3;

        std::vector<const char*> extensions;

        {
            uint32_t glfw_ext_count = 0;
            const char** glfw_extensions;

            glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_ext_count);


            for (uint32_t i = 0; i < glfw_ext_count; ++i)
            {
                extensions.push_back(glfw_extensions[i]);
            }

            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        vk::InstanceCreateInfo instance_create = {};
        instance_create.pApplicationInfo = &app_info;
        instance_create.setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()));
        instance_create.setPpEnabledExtensionNames(extensions.data());

        vk::DebugUtilsMessengerCreateInfoEXT debug_create_info = {};
        std::array validation_layers = { "VK_LAYER_KHRONOS_validation" };
        if (Util::s_EnableValidationLayer)
        {
            debug_create_info.setMessageSeverity
            (
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
            );
            debug_create_info.setMessageType
            (
                vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral    |
                vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance
            );
            debug_create_info.setPfnUserCallback(Util::VK_DebugCallback);

            instance_create.setPNext(&debug_create_info);
            instance_create.setEnabledLayerCount(static_cast<uint32_t>(validation_layers.size()));
            instance_create.setPpEnabledLayerNames(validation_layers.data());
        }

        try
        {
            m_Instance.emplace(*m_Context, instance_create);
        }
        catch (const vk::SystemError& e)
        {
            auto result = static_cast<vk::Result>(e.code().value());
            BC_CORE_ERROR("VulkanCore::InitInstance: Failed to Create Unique Instance.");
            return result;
        }

        return vk::Result::eSuccess;
    }

    vk::Result VulkanCore::InitDebugMessenger()
    {
        if (!Util::s_EnableValidationLayer)
        {
            return vk::Result::eSuccess;
        }

        vk::DebugUtilsMessengerCreateInfoEXT debug_create_info = {};
        debug_create_info.messageSeverity = 
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | 
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | 
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;

        debug_create_info.messageType = 
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral      |
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation   |
            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

        debug_create_info.pfnUserCallback = Util::VK_DebugCallback,
        debug_create_info.pUserData = VK_NULL_HANDLE;

        try
        {
            m_DebugMessenger.emplace(*m_Instance, debug_create_info);
        }
        catch (const vk::SystemError& e)
        {
            auto result = static_cast<vk::Result>(e.code().value());
            BC_CORE_ERROR("VulkanCore::InitDebugMessenger: Failed to Create Debug Messenger.");
            return result;
        }
        
        return vk::Result::eSuccess;
    }

    vk::Result VulkanCore::InitSurface(GLFWwindow *window)
    {
        VkSurfaceKHR raw_surface;
        if (auto result = glfwCreateWindowSurface(*(*m_Instance), window, VK_NULL_HANDLE, &raw_surface); result != VK_SUCCESS)
        {
            BC_CORE_ERROR("VulkanCore::InitSurface: Failed to Create Window Surface.");
            return static_cast<vk::Result>(result);
        }

        m_Surface.emplace(*m_Instance, raw_surface);

        return vk::Result::eSuccess;
    }

    vk::Result VulkanCore::InitDevice()
    {
        // 1. Select Physical Device
        {
            m_PhysicalDevices = m_Instance->enumeratePhysicalDevices();
            BC_THROW(!m_PhysicalDevices.empty(), "VulkanCore::InitDevice: No Physical Devices Found.");

            struct DeviceCandidate
            {
                vk::raii::PhysicalDevice* device;
                int score;
            };
            
            auto SupportsExtensions = [](const vk::raii::PhysicalDevice& device, const std::vector<const char*>& required_extensions) -> bool
            {
                auto available_extensions = device.enumerateDeviceExtensionProperties();
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
                    if (!found) return false;
                }
                return true;
            };
            
            auto HasRequiredQueues = [&](const vk::raii::PhysicalDevice& device) -> bool 
            {
                bool has_graphics = false, has_compute = false, has_transfer = false, has_present = false;
                auto queue_families = device.getQueueFamilyProperties();
                for (uint32_t i = 0; i < queue_families.size(); ++i) 
                {
                    const auto& family = queue_families[i];
                    if (family.queueFlags & vk::QueueFlagBits::eGraphics)
                        has_graphics = true;
                    if (family.queueFlags & vk::QueueFlagBits::eCompute)
                        has_compute = true;
                    if (family.queueFlags & vk::QueueFlagBits::eTransfer)
                        has_transfer = true;
                    if (device.getSurfaceSupportKHR(i, *m_Surface))
                        has_present = true;
                }
                return has_graphics && has_compute && has_transfer && has_present;
            };
            
            std::vector<DeviceCandidate> candidates;
            const std::vector<const char*> required_extensions = 
            {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                VK_KHR_MAINTENANCE1_EXTENSION_NAME,
                VK_KHR_MAINTENANCE2_EXTENSION_NAME,
                VK_KHR_MAINTENANCE3_EXTENSION_NAME
            };
            
            for (auto& device : m_PhysicalDevices) 
            {
                vk::PhysicalDeviceProperties properties = device.getProperties();
                vk::PhysicalDeviceFeatures features = device.getFeatures();
            
                int score = 0;
            
                // Prefer discrete GPUs
                if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
                    score += 1000;
            
                // Check for required queue families
                if (!HasRequiredQueues(device))
                    continue;
                
                score += 100; // Add score for having required queues
            
                // Check for required extensions
                if (!SupportsExtensions(device, required_extensions))
                    continue;

                score += 50; // Add score for having required extensions
            
                candidates.push_back({ &device, score });
            }
            
            BC_THROW(!candidates.empty(), "VulkanCore::InitDevice: No Suitable Physical Device Found.");
            
            // Pick the best device
            std::sort(candidates.begin(), candidates.end(),
                [](const DeviceCandidate& a, const DeviceCandidate& b) { return a.score > b.score; });

            m_SelectedDeviceIndex = -1;
            for (size_t i = 0; i < m_PhysicalDevices.size(); ++i) 
            {
                if (&m_PhysicalDevices[i] == candidates[0].device) 
                {
                    m_SelectedDeviceIndex = i;
                    break;
                }
            }
            BC_THROW(m_SelectedDeviceIndex >= 0 && m_SelectedDeviceIndex < static_cast<int>(m_PhysicalDevices.size()), "VulkanCore::InitDevice: Selected Device Index Out Of Range.");
        }

        auto& physical_device = GetPhysicalDevice();

        // 2. Set Queue Family Indices
        {
            auto queue_families = physical_device.getQueueFamilyProperties();

            for (size_t i = 0; i < queue_families.size(); ++i) 
            {
                const auto& family = queue_families[i];

                if (family.queueFlags & vk::QueueFlagBits::eGraphics && !m_QueueFamilyIndices.graphics_family.has_value()) 
                    m_QueueFamilyIndices.graphics_family.emplace(static_cast<uint32_t>(i));
                
                if (family.queueFlags & vk::QueueFlagBits::eCompute && !m_QueueFamilyIndices.compute_family.has_value())
                    m_QueueFamilyIndices.compute_family.emplace(static_cast<uint32_t>(i));

                if (family.queueFlags & vk::QueueFlagBits::eTransfer && !m_QueueFamilyIndices.transfer_family.has_value())
                    m_QueueFamilyIndices.transfer_family.emplace(static_cast<uint32_t>(i));

                vk::Bool32 present_supported = physical_device.getSurfaceSupportKHR(i, *m_Surface);
                if (present_supported && !m_QueueFamilyIndices.present_family.has_value())
                    m_QueueFamilyIndices.present_family.emplace(static_cast<uint32_t>(i));
            }

            if (!m_QueueFamilyIndices.graphics_family.has_value()   || 
                !m_QueueFamilyIndices.compute_family.has_value()    || 
                !m_QueueFamilyIndices.present_family.has_value()    || 
                !m_QueueFamilyIndices.transfer_family.has_value())
            {
                BC_CORE_ERROR("VulkanCore::InitDevice: Device Queues Not Contain All Functionality.");
                return vk::Result::eErrorInitializationFailed;
            }
        }

        // 3. Initialise Logical Device and Queues
        {
            // Set up queue priorities and create queue create infos
            std::array queue_priority = { 1.0f };
            std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
            std::set<uint32_t> unique_queue_families = 
            { 
                m_QueueFamilyIndices.graphics_family.value(), 
                m_QueueFamilyIndices.present_family.value(), 
                m_QueueFamilyIndices.compute_family.value(), 
                m_QueueFamilyIndices.transfer_family.value() 
            };

            for (uint32_t queue_family : unique_queue_families) 
            {
                vk::DeviceQueueCreateInfo queue_create = {};

                queue_create.setQueueFamilyIndex(queue_family);
                queue_create.setQueueCount(1);
                queue_create.setPQueuePriorities(queue_priority.data());

                queue_create_infos.push_back(queue_create);
            }

            vk::DeviceCreateInfo device_create_info{};

            device_create_info.setQueueCreateInfoCount(static_cast<uint32_t>(queue_create_infos.size()));
            device_create_info.setPQueueCreateInfos(queue_create_infos.data());
            
            device_create_info.setEnabledLayerCount(Util::s_EnableValidationLayer ? 1 : 0);
            const char* validation_layer = "VK_LAYER_KHRONOS_validation";
            device_create_info.setPpEnabledLayerNames(&validation_layer);

            const std::vector<const char*> device_extensions = 
            { 
                VK_KHR_SWAPCHAIN_EXTENSION_NAME,
                VK_KHR_MAINTENANCE1_EXTENSION_NAME,
                VK_KHR_MAINTENANCE2_EXTENSION_NAME,
                VK_KHR_MAINTENANCE3_EXTENSION_NAME
            };

            device_create_info.setEnabledExtensionCount(static_cast<uint32_t>(device_extensions.size()));
            device_create_info.setPpEnabledExtensionNames(device_extensions.data());

            vk::PhysicalDeviceFeatures device_features = physical_device.getFeatures();
            device_create_info.setPEnabledFeatures(&device_features);

            try
            {
                m_LogicalDevice.emplace(physical_device, device_create_info);
            }
            catch (const vk::SystemError& e)
            {
                auto result = static_cast<vk::Result>(e.code().value());
                BC_CORE_ERROR("VulkanCore::InitDevice: Failed to Create Logical Device.");
                return result;
            }

            m_GraphicsQueue.emplace(*m_LogicalDevice, m_QueueFamilyIndices.graphics_family.value(), 0);
            m_ComputeQueue.emplace(*m_LogicalDevice, m_QueueFamilyIndices.compute_family.value(), 0);
            m_PresentQueue.emplace(*m_LogicalDevice, m_QueueFamilyIndices.present_family.value(), 0);
            m_TransferQueue.emplace(*m_LogicalDevice, m_QueueFamilyIndices.transfer_family.value(), 0);
        }
        
        return vk::Result::eSuccess;
    }

    vk::Result VulkanCore::InitVMA()
    {
        if (!m_LogicalDevice)
        {
            BC_CORE_ERROR("VulkanCore::InitVMA: Logical Device Not Valid.");
            return vk::Result::eErrorInitializationFailed;
        }

        VmaAllocatorCreateInfo allocator_info = {};
        allocator_info.physicalDevice = *GetPhysicalDevice();
        allocator_info.device = *GetLogicalDevice();
        allocator_info.instance = *GetInstance();
        allocator_info.vulkanApiVersion = VK_API_VERSION_1_3;

        VkResult result = vmaCreateAllocator(&allocator_info, &m_Allocator);
        if (result != VK_SUCCESS)   
        {
            BC_CORE_ERROR("VulkanCore::InitVMA: Failed to Create VMA Allocator.");
            return static_cast<vk::Result>(result);
        }
        
        return vk::Result::eSuccess;
    }

    vk::Result VulkanCore::InitSwapChainFramesInFlight()
    {
        BC_ASSERT(m_Swapchain, "VulkanCore::InitSwapChainFramesInFlight: SwapChain Needs to be Valid.");

        m_SwapChainFramesInFlight.resize(m_Swapchain->GetImageCount());

        vk::SemaphoreCreateInfo semaphore_info{};

        vk::FenceCreateInfo fence_info = {};
        fence_info.setFlags(vk::FenceCreateFlagBits::eSignaled); // Start signaled so first frame doesn't wait forever

        for (auto& frame : m_SwapChainFramesInFlight)
        {
            // Semaphores
            try
            {
                frame.image_available_semaphore.emplace(std::move(m_LogicalDevice->createSemaphore(semaphore_info)));
                frame.render_finished_semaphore.emplace(std::move(m_LogicalDevice->createSemaphore(semaphore_info)));
            }
            catch (const vk::SystemError& e)
            {
                auto result = static_cast<vk::Result>(e.code().value());
                BC_CORE_ERROR("VulkanCore::InitSwapChainFramesInFlight: Failed to Create Frame in Flight Semaphores.");
                return result;
            }

            // Fence
            try
            {
                frame.in_flight_fence.emplace(std::move(m_LogicalDevice->createFence(fence_info)));
            }
            catch (const vk::SystemError& e)
            {
                auto result = static_cast<vk::Result>(e.code().value());
                BC_CORE_ERROR("VulkanCore::InitSwapChainFramesInFlight: Failed to Create Frame in Flight Fence.");
                return result;
            }
            
            // Create Command Pool
            try
            {
                vk::CommandPoolCreateInfo pool_info{};
                pool_info.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
                pool_info.queueFamilyIndex = m_QueueFamilyIndices.graphics_family.value();

                frame.command_pool.emplace(std::move(m_LogicalDevice->createCommandPool(pool_info)));
            }
            catch (const vk::SystemError& e)
            {
                auto result = static_cast<vk::Result>(e.code().value());
                BC_CORE_ERROR("VulkanCore::InitSwapChainFramesInFlight: Failed to Create Frame in Flight Command Pool.");
                return result;
            }

            // Allocate command buffer
            try
            {
                vk::CommandBufferAllocateInfo alloc_info = {};
                alloc_info.commandPool = *frame.command_pool;
                alloc_info.level = vk::CommandBufferLevel::ePrimary;
                alloc_info.commandBufferCount = 1;

                frame.command_buffer.emplace(std::move(m_LogicalDevice->allocateCommandBuffers(alloc_info)[0]));
            }
            catch (const vk::SystemError& e)
            {
                auto result = static_cast<vk::Result>(e.code().value());
                BC_CORE_ERROR("VulkanCore::InitSwapChainFramesInFlight: Failed to Allocate Frame in Flight Command Buffer.");
                return result;
            }

            // Descriptor Pool
            try
            {
                vk::DescriptorPoolSize pool_sizes[] =
                {
                    { vk::DescriptorType::eSampler, 8192 },
                    { vk::DescriptorType::eCombinedImageSampler, 8192 },
                    { vk::DescriptorType::eSampledImage, 8192 },
                    { vk::DescriptorType::eStorageImage, 8192 },
                    { vk::DescriptorType::eUniformTexelBuffer, 8192 },
                    { vk::DescriptorType::eStorageTexelBuffer, 8192 },
                    { vk::DescriptorType::eUniformBuffer, 8192 },
                    { vk::DescriptorType::eStorageBuffer, 8192 },
                    { vk::DescriptorType::eUniformBufferDynamic, 8192 },
                    { vk::DescriptorType::eStorageBufferDynamic, 8192 },
                    { vk::DescriptorType::eInputAttachment, 8192 }
                };

                vk::DescriptorPoolCreateInfo pool_info = {};
                pool_info.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
                pool_info.setMaxSets(8192 * static_cast<uint32_t>(BC_ARRAY_SIZE(pool_sizes)));
                pool_info.setPoolSizeCount(static_cast<uint32_t>(BC_ARRAY_SIZE(pool_sizes)));
                pool_info.setPPoolSizes(pool_sizes);

                frame.descriptor_pool.emplace(*m_LogicalDevice, pool_info);
            }
            catch (const vk::SystemError& e)
            {
                auto result = static_cast<vk::Result>(e.code().value());
                BC_CORE_ERROR("VulkanCore::InitSwapChainFramesInFlight: Failed to Create Frame in Flight Descriptor Pool.");
                return result;
            }
        }

        return vk::Result::eSuccess;
    }

    vk::Result VulkanCore::InitSwapchain(const SwapchainSpecification& swapchain_spec)
    {
        BC_CATCH_BEGIN();

        m_Swapchain.reset();
        m_Swapchain = std::make_unique<Swapchain>(swapchain_spec);

        BC_CATCH_END_RETURN(vk::Result::eErrorInitializationFailed);

        std::string present_mode;
        switch(swapchain_spec.present_mode)
        {
            default:                                present_mode = "";              break;
            case vk::PresentModeKHR::eImmediate:    present_mode = "Immediate";     break;
            case vk::PresentModeKHR::eMailbox:      present_mode = "Mailbox";       break;
            case vk::PresentModeKHR::eFifo:         present_mode = "FIFO";          break;
            case vk::PresentModeKHR::eFifoRelaxed:  present_mode = "FIFO Relaxed";  break;
        }
        BC_CORE_TRACE("VulkanCore::InitSwapchain: Created Swapchain - Present Mode {}.", present_mode);
        
        return vk::Result::eSuccess;
    }

#pragma endregion

#pragma region Frame Management

    void VulkanCore::BeginFrame()
    {
        BC_THROW(!m_SwapChainFramesInFlight.empty(), "VulkanCore::BeginFrame: Frame Sync in Flight Vector Empty.");

        auto& frame = m_SwapChainFramesInFlight[m_FrameIndex];

        vk::Result result = m_LogicalDevice->waitForFences({*frame.in_flight_fence}, VK_TRUE, UINT64_MAX);
        BC_THROW(result == vk::Result::eSuccess, "VulkanCore::BeginFrame: Could Not Wait For Fences.");

        try
        {
            m_LogicalDevice->resetFences({*frame.in_flight_fence});
        }
        catch (const vk::SystemError& e)
        {
            BC_THROW(false, std::format("VulkanCore::BeginFrame: Failed to Reset Fences - {}.", e.what()));
            return;
        }

        frame.command_pool.value().reset();

        try
        {
            auto [result, image_index] = m_Swapchain->GetSwapchainKHR().acquireNextImage(
                UINT64_MAX,
                *frame.image_available_semaphore,
                VK_NULL_HANDLE
            );
            m_CurrentImageIndex = image_index;
            frame.image_index = m_CurrentImageIndex;
        }
        catch (const vk::OutOfDateKHRError& e)
        {
            BC_CORE_WARN("VulkanCore::BeginFrame: Swapchain Out of Date - {}", e.what());
            Application::Get()->Close();
            return;
        }
        catch (const vk::SystemError& e)
        {
            BC_THROW(false, std::format("VulkanCore::BeginFrame: Failed to Acquire Next Image - {}.", e.what()));
            return;
        }

        try
        {
            frame.command_buffer->begin(vk::CommandBufferBeginInfo{});
        }
        catch (const vk::SystemError& e)
        {
            BC_THROW(false, std::format("VulkanCore::BeginFrame: Could Not Begin Command Buffer - {}.", e.what()));
            return;
        }
    }
    
    void VulkanCore::EndFrame()
    {
        auto& frame = m_SwapChainFramesInFlight[m_FrameIndex];
    
        frame.command_buffer->end();
    
        vk::PipelineStageFlags wait_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    
        vk::SubmitInfo submit_info = {};
        submit_info.setWaitSemaphoreCount(1);
        submit_info.setPWaitSemaphores(&(**frame.image_available_semaphore));
        submit_info.setPWaitDstStageMask(&wait_stage);
        submit_info.setCommandBufferCount(1);
        submit_info.setPCommandBuffers(&(**frame.command_buffer));
        submit_info.setSignalSemaphoreCount(1);
        submit_info.setPSignalSemaphores(&(**frame.render_finished_semaphore));

        try
        {
            m_GraphicsQueue->submit({ submit_info }, **frame.in_flight_fence);
        }
        catch (const vk::SystemError& e)
        {
            BC_THROW(false, std::format("VulkanCore::EndFrame: Could Not Submit Command Buffer - {}.", e.what()));
            return;
        }
    
        vk::PresentInfoKHR present_info = {};
        present_info.setWaitSemaphoreCount(1);
        present_info.setPWaitSemaphores(&(**frame.render_finished_semaphore));
        present_info.setSwapchainCount(1);
        present_info.setPSwapchains(&(*m_Swapchain->GetSwapchainKHR()));
        present_info.setPImageIndices(&m_CurrentImageIndex);

        try
        {
            auto result = m_PresentQueue->presentKHR(present_info);
            if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
            {
                BC_THROW(false, std::format("VulkanCore::EndFrame: Failed to Present - {}.", vk::to_string(result)));
                return;
            }
        }
        catch (const vk::OutOfDateKHRError& e)
        {
            BC_CORE_WARN("VulkanCore::EndFrame: Swapchain Out of Date - {}", e.what());
            Application::Get()->Close();
            return;
        }
        catch (const vk::SystemError& e)
        {
            BC_THROW(false, std::format("VulkanCore::EndFrame: Failed to Present - {}.", e.what()));
            return;
        }
    
        m_FrameIndex = (m_FrameIndex + 1) % m_SwapChainFramesInFlight.size();
    }

    void VulkanCore::BeginSwapchainRenderPass()
    {
        auto& frame = m_SwapChainFramesInFlight[m_FrameIndex];

        vk::RenderPassBeginInfo render_pass_info = {};
        render_pass_info.setRenderPass(m_Swapchain->GetRenderPass());
        render_pass_info.setFramebuffer(*m_Swapchain->GetFramebuffers()[frame.image_index]);
        render_pass_info.setRenderArea(vk::Rect2D(vk::Offset2D{ 0, 0 }, m_Swapchain->GetSpecification().extent));

        vk::ClearValue clear_color = {};
        clear_color.color = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);

        render_pass_info.setClearValueCount(1);
        render_pass_info.setPClearValues(&clear_color);

        frame.command_buffer->beginRenderPass(render_pass_info, vk::SubpassContents::eInline);
    }

    void VulkanCore::EndSwapchainRenderPass()
    {
        m_SwapChainFramesInFlight[m_FrameIndex].command_buffer->endRenderPass();
    }

#pragma endregion

#pragma region Getters

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

    uint32_t VulkanCore::GetTransferQueueFamily() const
    {
        if (!m_QueueFamilyIndices.transfer_family.has_value())
            return -1;

        return m_QueueFamilyIndices.transfer_family.value();
    }

#pragma endregion

#pragma region Resizing

    void VulkanCore::ResizeScreenSpace(uint32_t width, uint32_t height)
    {
        // TODO: Do we need to do anything here?
    }

    void VulkanCore::ResizeSwapchain(const SwapchainSpecification& swapchain_spec)
    {
        BC_CATCH_BEGIN();

        m_LogicalDevice->waitIdle();

        m_Swapchain.reset();
        m_Swapchain = std::make_unique<Swapchain>(swapchain_spec);

        BC_CATCH_END_FUNC([]() { Application::Get()->Close(); });
    }

#pragma endregion

#pragma region General Functions

    uint32_t VulkanCore::FindMemoryType(uint32_t type_filter, vk::MemoryPropertyFlags properties) const
    {
        auto physical_device = GetPhysicalDevice();

        vk::PhysicalDeviceMemoryProperties mem_properties = physical_device.getMemoryProperties();

        for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++)
        {
            if ((type_filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties)
            {
                return i;
            }
        }

        BC_THROW(false, "VulkanCore::FindMemoryType: Failed to find suitable memory type!");
    }

#pragma endregion

#pragma region Single Use Command Buffers

    vk::raii::CommandPool& VulkanCore::GetThreadCommandPool()
    {
        std::lock_guard<std::mutex> lock(m_UploadPoolMutex);

        auto thread_id = std::this_thread::get_id();
        auto it = m_ThreadUploadPools.find(thread_id);
        if (it != m_ThreadUploadPools.end())
            return it->second;

        vk::CommandPoolCreateInfo pool_info = {};
        pool_info.setQueueFamilyIndex(m_QueueFamilyIndices.graphics_family.value());
        pool_info.setFlags
        (
            vk::CommandPoolCreateFlagBits::eTransient | 
            vk::CommandPoolCreateFlagBits::eResetCommandBuffer
        );

        auto command_pool = m_LogicalDevice->createCommandPool(pool_info);
        m_ThreadUploadPools.emplace(thread_id, std::move(command_pool));

        return m_ThreadUploadPools.at(thread_id);
    }

    vk::raii::CommandBuffer VulkanCore::BeginSingleUseCommandBuffer(vk::raii::CommandPool& pool)
    {
        vk::CommandBufferAllocateInfo alloc_info = {};
        alloc_info.setLevel(vk::CommandBufferLevel::ePrimary);
        alloc_info.setCommandPool(pool);
        alloc_info.setCommandBufferCount(1);

        vk::raii::CommandBuffer cmd_buffer = std::move(m_LogicalDevice->allocateCommandBuffers(alloc_info)[0]);

        cmd_buffer.begin(vk::CommandBufferBeginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
        return std::move(cmd_buffer);
    }

    void VulkanCore::EndSingleUseCommandBuffer(vk::raii::Queue& queue, vk::raii::CommandPool& pool, vk::raii::CommandBuffer& cmd)
    {
        cmd.end();
    
        vk::SubmitInfo submit_info{};
        submit_info.setCommandBufferCount(1);
        submit_info.setPCommandBuffers(&(*cmd));
    
        vk::FenceCreateInfo fence_info{};
        vk::raii::Fence fence(*m_LogicalDevice, fence_info);
    
        queue.submit({ submit_info }, *fence);
        BC_THROW(m_LogicalDevice->waitForFences({ *fence }, VK_TRUE, UINT64_MAX) == vk::Result::eSuccess, "VulkanCore::EndSingleUseCommandBuffer: Failed to Wait for Fence.");
    }

#pragma endregion
    
}