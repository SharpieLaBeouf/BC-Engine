#include "VulkanUtil.h"

#include "Core/Application.h"

namespace BC::Util
{
    void TransitionImageLayout(vk::raii::CommandBuffer& command_buffer, vk::Image image, vk::Format format, vk::ImageLayout old_layout, vk::ImageLayout new_layout, uint32_t mip_levels)
    {
        vk::ImageMemoryBarrier barrier{};
        barrier.setOldLayout(old_layout);
        barrier.setNewLayout(new_layout);
        barrier.setSrcQueueFamilyIndex(vk::QueueFamilyIgnored);
        barrier.setDstQueueFamilyIndex(vk::QueueFamilyIgnored);
        barrier.setImage(image);
        barrier.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor);
        barrier.subresourceRange.setBaseMipLevel(0);
        barrier.subresourceRange.setLevelCount(mip_levels);
        barrier.subresourceRange.setBaseArrayLayer(0);
        barrier.subresourceRange.setLayerCount(1);

        vk::PipelineStageFlags src_stage;
        vk::PipelineStageFlags dst_stage;

        if (old_layout == vk::ImageLayout::eUndefined && new_layout == vk::ImageLayout::eTransferDstOptimal)
        {
            barrier.setSrcAccessMask(vk::AccessFlagBits::eNone);
            barrier.setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
            src_stage = vk::PipelineStageFlagBits::eTopOfPipe;
            dst_stage = vk::PipelineStageFlagBits::eTransfer;
        }
        else if (old_layout == vk::ImageLayout::eTransferDstOptimal && new_layout == vk::ImageLayout::eShaderReadOnlyOptimal)
        {
            barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
            barrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead);
            src_stage = vk::PipelineStageFlagBits::eTransfer;
            dst_stage = vk::PipelineStageFlagBits::eFragmentShader;
        }
        else
        {
            // Handle other layout transitions as needed
            BC_THROW(false, "TransitionImageLayout: Unsupported layout transition!");
        }

        command_buffer.pipelineBarrier(
            src_stage, dst_stage,
            vk::DependencyFlags(),
            {}, {}, // No memory/buffer barriers
            barrier);
    }

    void CopyBufferToImage(vk::raii::CommandBuffer& command_buffer, vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height)
    {
        vk::BufferImageCopy region = {};
        region.setBufferOffset(0);
        region.setBufferRowLength(0);
        region.setBufferImageHeight(0);

        region.imageSubresource.setAspectMask(vk::ImageAspectFlagBits::eColor);
        region.imageSubresource.setMipLevel(0);
        region.imageSubresource.setBaseArrayLayer(0);
        region.imageSubresource.setLayerCount(1);

        region.setImageOffset({ 0, 0, 0 });
        region.setImageExtent({
            width,
            height,
            1
        });

        command_buffer.copyBufferToImage(
            buffer,
            image,
            vk::ImageLayout::eTransferDstOptimal,
            region);
    }

    void GenerateMipmaps(vk::raii::CommandBuffer& command_buffer, vk::Image image, vk::Format format, int32_t width, int32_t height, uint32_t mip_levels)
    {
        int32_t mip_width = width;
        int32_t mip_height = height;

        for (uint32_t i = 1; i < mip_levels; ++i)
        {
            vk::ImageMemoryBarrier barrier{};
            barrier.setImage(image);
            barrier.setSrcQueueFamilyIndex(vk::QueueFamilyIgnored);
            barrier.setDstQueueFamilyIndex(vk::QueueFamilyIgnored);
            barrier.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor);
            barrier.subresourceRange.setBaseArrayLayer(0);
            barrier.subresourceRange.setLayerCount(1);
            barrier.subresourceRange.setLevelCount(1);
            barrier.subresourceRange.setBaseMipLevel(i - 1);

            barrier.setOldLayout(vk::ImageLayout::eTransferDstOptimal);
            barrier.setNewLayout(vk::ImageLayout::eTransferSrcOptimal);
            barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
            barrier.setDstAccessMask(vk::AccessFlagBits::eTransferRead);

            command_buffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eTransfer,
                vk::PipelineStageFlagBits::eTransfer,
                vk::DependencyFlags(),
                {}, {}, // No memory/buffer barriers
                barrier
            );

            vk::ImageBlit blit = {};
            blit.srcOffsets[0] = vk::Offset3D{ 0, 0, 0 };
            blit.srcOffsets[1] = vk::Offset3D{ mip_width, mip_height, 1 };
            blit.srcSubresource.setAspectMask(vk::ImageAspectFlagBits::eColor);
            blit.srcSubresource.setMipLevel(i - 1);
            blit.srcSubresource.setBaseArrayLayer(0);
            blit.srcSubresource.setLayerCount(1);

            blit.dstOffsets[0] = vk::Offset3D{ 0, 0, 0 };
            blit.dstOffsets[1] = vk::Offset3D{
                mip_width > 1 ? mip_width / 2 : 1,
                mip_height > 1 ? mip_height / 2 : 1,
                1
            };
            blit.dstSubresource.setAspectMask(vk::ImageAspectFlagBits::eColor);
            blit.dstSubresource.setMipLevel(i);
            blit.dstSubresource.setBaseArrayLayer(0);
            blit.dstSubresource.setLayerCount(1);

            command_buffer.blitImage(
                image, vk::ImageLayout::eTransferSrcOptimal,
                image, vk::ImageLayout::eTransferDstOptimal,
                blit,
                vk::Filter::eLinear
            );

            barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
            barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

            command_buffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
                vk::DependencyFlags(),
                {}, {}, // No memory/buffer barriers
                barrier
            );

            if (mip_width > 1) mip_width /= 2;
            if (mip_height > 1) mip_height /= 2;
        }

        vk::ImageMemoryBarrier final_barrier = {};
        final_barrier.setOldLayout(vk::ImageLayout::eTransferDstOptimal);
        final_barrier.setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
        final_barrier.setSrcQueueFamilyIndex(vk::QueueFamilyIgnored);
        final_barrier.setDstQueueFamilyIndex(vk::QueueFamilyIgnored);
        final_barrier.setImage(image);
        final_barrier.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor);
        final_barrier.subresourceRange.setBaseMipLevel(mip_levels - 1);
        final_barrier.subresourceRange.setLevelCount(1);
        final_barrier.subresourceRange.setBaseArrayLayer(0);
        final_barrier.subresourceRange.setLayerCount(1);
        final_barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
        final_barrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead);

        command_buffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
            vk::DependencyFlags(),
            {}, {}, // No memory/buffer barriers
            final_barrier
        );
    }

}