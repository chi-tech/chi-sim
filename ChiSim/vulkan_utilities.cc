#include "chi_sim.h"

//###################################################################
/** Find buffer memory type.*/
uint32_t ChiSim::FindMemoryType(uint32_t typeFilter,
                                VkMemoryPropertyFlags properties)
{
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(m_physical_device, &memProperties);

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
    if ((typeFilter & (1 << i)) &&
        (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
      return i;

  throw std::runtime_error("failed to find suitable memory type!");
}

//###################################################################
/** Begin single time commands. */
VkCommandBuffer ChiSim::BeginSingleTimeCommands()
{
  VkCommandBufferAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = m_command_pool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(commandBuffer, &beginInfo);

  return commandBuffer;
}

//###################################################################
/** End single time commands. */
void ChiSim::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
{
  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  vkQueueSubmit(m_graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(m_graphics_queue);

  vkFreeCommandBuffers(m_device, m_command_pool, 1, &commandBuffer);
}

//###################################################################
/** Create image view. */
VkImageView ChiSim::CreateImageView(VkImage image,
                                    VkFormat format,
                                    VkImageAspectFlags aspect_flags)
{
  VkImageViewCreateInfo viewInfo = {};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = format;
  viewInfo.subresourceRange.aspectMask = aspect_flags;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  VkImageView imageView;
  if (vkCreateImageView(m_device,
                        &viewInfo,
                        nullptr,
                        &imageView) != VK_SUCCESS)
    throw std::runtime_error("failed to create texture image view!");

  return imageView;
}

bool ChiSim::HasStencilComponent(VkFormat format)
{
  return format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
         format == VK_FORMAT_D24_UNORM_S8_UINT;
}

