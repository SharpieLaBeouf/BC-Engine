#pragma once

#include "Debug/Logging.h"
#include "Debug/Assert.h"

#include <vulkan/vulkan_raii.hpp>

namespace BC
{
    #define VK_CHECK_ERROR(expr, msg)                                           \
    do {                                                                        \
        vk::Result _res = (expr);                                               \
        if (_res != vk::Result::eSuccess) {                                     \
            BC_CORE_ERROR("[Vulkan Error] {} | {} at {}:{}",                    \
                vk::to_string(_res), (msg), __FILE__, __LINE__);                \
            BC_ASSERT(false, msg);                                              \
        }                                                                       \
    } while (0)

    namespace Util
    {
        #ifdef NDEBUG
            static bool s_EnableValidationLayer = false;
        #else
            static bool s_EnableValidationLayer = true;
        #endif

        static bool IsValidationLayerSupported()
        {
            auto available_layers = vk::enumerateInstanceLayerProperties();

            for (const auto& properties : available_layers) 
            {
                if (strcmp("VK_LAYER_KHRONOS_validation", properties.layerName) == 0) 
                {
                    return true;
                }
            }

            return false;
        }

        static VKAPI_ATTR vk::Bool32 VKAPI_CALL VK_DebugCallback(
            vk::DebugUtilsMessageSeverityFlagBitsEXT message_severity, 
            vk::DebugUtilsMessageTypeFlagsEXT message_type, 
            const vk::DebugUtilsMessengerCallbackDataEXT* p_callback_data, 
            void* p_user_data) 
        {
            if (message_severity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
            {
                BC_CORE_ERROR("Vulkan Validation Error: {}", p_callback_data->pMessage);
            }
            else if (message_severity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
            {
                BC_CORE_WARN("Vulkan Validation Warning: {}", p_callback_data->pMessage);
            }
            else if (message_severity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo)
            {
                BC_CORE_INFO("Vulkan Validation Info: {}", p_callback_data->pMessage);
            }
            else if (message_severity & vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose)
            {
                BC_CORE_TRACE("Vulkan Validation Trace: {}", p_callback_data->pMessage);
            }

            return VK_FALSE;
        }

        void TransitionImageLayout(
            vk::raii::CommandBuffer& command_buffer,
            vk::Image image,
            vk::Format format,
            vk::ImageLayout old_layout,
            vk::ImageLayout new_layout,
            uint32_t mip_levels);

        void CopyBufferToImage(
            vk::raii::CommandBuffer& command_buffer,
            vk::Buffer buffer,
            vk::Image image,
            uint32_t width,
            uint32_t height);

        void GenerateMipmaps(
            vk::raii::CommandBuffer& command_buffer,
            vk::Image image,
            vk::Format format,
            int32_t width,
            int32_t height,
            uint32_t mip_levels);
    
    }
}