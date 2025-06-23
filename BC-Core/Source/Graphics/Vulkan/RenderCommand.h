#pragma once

// Core Headers

// C++ Standard Library Headers
#include <vector>

// External Vendor Library Headers
#include <vulkan/vulkan.h>

namespace BC
{

    class RenderCommandBuffer
    {

    public:

        RenderCommandBuffer() = default;
        ~RenderCommandBuffer() = default;

        RenderCommandBuffer(const RenderCommandBuffer&) = delete;
        RenderCommandBuffer& operator=(const RenderCommandBuffer&) = delete;

        RenderCommandBuffer(RenderCommandBuffer&& other) noexcept
        {
            *this = std::move(other);
        }

        RenderCommandBuffer& operator=(RenderCommandBuffer&& other) noexcept
        {
            command_buffer = other.command_buffer;
            queue = other.queue;
            signal_fence = other.signal_fence;
            wait_semaphores = std::move(other.wait_semaphores);
            signal_semaphores = std::move(other.signal_semaphores);
            wait_stage_mask = other.wait_stage_mask;

            other.command_buffer = VK_NULL_HANDLE;
            other.queue = VK_NULL_HANDLE;
            other.signal_fence = VK_NULL_HANDLE;
            other.wait_stage_mask = 0;

            return *this;
        }

        // Factory method for basic command buffer submission
        static RenderCommandBuffer Create(
            VkCommandBuffer cmd_buffer,
            VkQueue target_queue,
            VkFence fence = VK_NULL_HANDLE,
            std::vector<VkSemaphore> wait_sems = {},
            std::vector<VkSemaphore> signal_sems = {},
            VkPipelineStageFlags wait_stages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        )
        {
            RenderCommandBuffer cmd;
            cmd.command_buffer = cmd_buffer;
            cmd.queue = target_queue;
            cmd.signal_fence = fence;
            cmd.wait_semaphores = std::move(wait_sems);
            cmd.signal_semaphores = std::move(signal_sems);
            cmd.wait_stage_mask = wait_stages;
            return cmd;
        }

        // Submission helper
        void Submit() const
        {
            VkSubmitInfo submit_info{};
            submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

            submit_info.commandBufferCount = 1;
            submit_info.pCommandBuffers = &command_buffer;

            submit_info.waitSemaphoreCount = static_cast<uint32_t>(wait_semaphores.size());
            submit_info.pWaitSemaphores = wait_semaphores.data();
            submit_info.pWaitDstStageMask = (wait_semaphores.empty()) ? nullptr : &wait_stage_mask;

            submit_info.signalSemaphoreCount = static_cast<uint32_t>(signal_semaphores.size());
            submit_info.pSignalSemaphores = signal_semaphores.data();

            vkQueueSubmit(queue, 1, &submit_info, signal_fence);
        }

    public:

        VkCommandBuffer command_buffer = VK_NULL_HANDLE;
        VkQueue queue = VK_NULL_HANDLE;
        VkFence signal_fence = VK_NULL_HANDLE;
        std::vector<VkSemaphore> wait_semaphores;
        std::vector<VkSemaphore> signal_semaphores;
        VkPipelineStageFlags wait_stage_mask = 0;

    };

}