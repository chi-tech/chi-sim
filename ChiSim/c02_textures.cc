#include "chi_sim.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

//###################################################################
/** Create texture image. */
void ChiSim::CreateTextureImage()
{
  int texWidth, texHeight, texChannels;
  stbi_uc* pixels = stbi_load("../textures/texture.jpg",
                              &texWidth,
                              &texHeight,
                              &texChannels,
                              STBI_rgb_alpha);
  VkDeviceSize imageSize = texWidth * texHeight * 4;

  if (!pixels)
    throw std::runtime_error("failed to load texture image!");

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;

  CreateBuffer(imageSize,
               VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               stagingBuffer,
               stagingBufferMemory);

  void* data;
  vkMapMemory(m_device, stagingBufferMemory, 0, imageSize, 0, &data);
  memcpy(data, pixels, static_cast<size_t>(imageSize));
  vkUnmapMemory(m_device, stagingBufferMemory);

  stbi_image_free(pixels);

  CreateImage(texWidth, texHeight,
              VK_FORMAT_R8G8B8A8_SRGB,
              VK_IMAGE_TILING_OPTIMAL,
              VK_IMAGE_USAGE_TRANSFER_DST_BIT |
              VK_IMAGE_USAGE_SAMPLED_BIT,
              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
              m_texture_image,
              m_texture_image_memory);

  TransitionImageLayout(m_texture_image,
                        VK_FORMAT_R8G8B8A8_SRGB,
                        VK_IMAGE_LAYOUT_UNDEFINED,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  CopyBufferToImage(stagingBuffer,
                    m_texture_image,
                    static_cast<uint32_t>(texWidth),
                    static_cast<uint32_t>(texHeight));
  TransitionImageLayout(m_texture_image,
                        VK_FORMAT_R8G8B8A8_SRGB,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  vkDestroyBuffer(m_device, stagingBuffer, nullptr);
  vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}


//###################################################################
/** Create texture image view. */
void ChiSim::CreateTextureImageView()
{
  m_texture_image_view =
    CreateImageView(m_texture_image, VK_FORMAT_R8G8B8A8_SRGB);
}

//###################################################################
/** Transition image layout. */
void ChiSim::TransitionImageLayout(VkImage image,
                                   VkFormat format,
                                   VkImageLayout oldLayout,
                                   VkImageLayout newLayout)
{
  VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

  VkImageMemoryBarrier barrier = {};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = oldLayout;
  barrier.newLayout = newLayout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = image;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;

  VkPipelineStageFlags sourceStage;
  VkPipelineStageFlags destinationStage;

  if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
      newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
  {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  }
  else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
           newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
  {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  }
  else
  {
    throw std::invalid_argument("unsupported layout transition!");
  }

  vkCmdPipelineBarrier(
    commandBuffer,
    sourceStage, destinationStage,
    0,
    0, nullptr,
    0, nullptr,
    1, &barrier
  );

  EndSingleTimeCommands(commandBuffer);
}

//###################################################################
/** Copy buffer to image. */
void ChiSim::CopyBufferToImage(VkBuffer buffer,
                               VkImage image,
                               uint32_t width,
                               uint32_t height)
{
  VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

  VkBufferImageCopy region = {};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;
  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;
  region.imageOffset = {0, 0, 0};
  region.imageExtent = { width, height, 1 };

  vkCmdCopyBufferToImage(commandBuffer,
                         buffer,
                         image,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

  EndSingleTimeCommands(commandBuffer);
}