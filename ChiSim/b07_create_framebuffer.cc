#include "chi_sim.h"

//###################################################################
/** Create a texture sampler. */
void ChiSim::CreateTextureSampler()
{
  VkSamplerCreateInfo samplerInfo = {};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.anisotropyEnable = VK_TRUE;
  samplerInfo.maxAnisotropy = 16;
  samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  samplerInfo.unnormalizedCoordinates = VK_FALSE;
  samplerInfo.compareEnable = VK_FALSE;
  samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerInfo.mipLodBias = 0.0f;
  samplerInfo.minLod = 0.0f;
  samplerInfo.maxLod = 0.0f;

  if (vkCreateSampler(m_device,
                      &samplerInfo,
                      nullptr,
                      &m_texture_sampler) != VK_SUCCESS)
    throw std::runtime_error("failed to create texture sampler!");
}

//###################################################################
/** Create depth resources. */
void ChiSim::CreateDepthResources()
{
  VkFormat depthFormat = FindDepthFormat();

  CreateImage(m_swap_chain_extent.width,
              m_swap_chain_extent.height,
              depthFormat,
              VK_IMAGE_TILING_OPTIMAL,
              VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
              m_depth_image, m_depth_image_memory);
  m_depth_image_view = CreateImageView(m_depth_image,
                                       depthFormat,
                                       VK_IMAGE_ASPECT_DEPTH_BIT);
}

//###################################################################
/** Finds a format supporting depth buffering. */
VkFormat ChiSim::FindSupportedFormat(
  const std::vector<VkFormat> &candidates,
  VkImageTiling tiling,
  VkFormatFeatureFlags features)
{
  for (VkFormat format : candidates)
  {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(m_physical_device, format, &props);

    if (tiling == VK_IMAGE_TILING_LINEAR &&
        (props.linearTilingFeatures & features) == features)
    {
      return format;
    } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
               (props.optimalTilingFeatures & features) == features)
    {
      return format;
    }
  }

  throw std::runtime_error("failed to find supported format!");
}

//###################################################################
/** Finds a depth format. */
VkFormat ChiSim::FindDepthFormat()
{
  return FindSupportedFormat(
    {VK_FORMAT_D32_SFLOAT,
     VK_FORMAT_D32_SFLOAT_S8_UINT,
     VK_FORMAT_D24_UNORM_S8_UINT},
    VK_IMAGE_TILING_OPTIMAL,
    VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
  );
}

//###################################################################
/** Create frame buffer. */
void ChiSim::CreateFramebuffers()
{
  m_swap_chain_framebuffers.resize(m_swap_chain_image_views.size());

  for (size_t i = 0; i < m_swap_chain_image_views.size(); i++)
  {
    std::array<VkImageView, 2> attachments =
      { m_swap_chain_image_views[i], m_depth_image_view};

    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = m_render_pass;
    framebufferInfo.attachmentCount = attachments.size();
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = m_swap_chain_extent.width;
    framebufferInfo.height = m_swap_chain_extent.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(m_device,
                            &framebufferInfo,
                            nullptr,
                            &m_swap_chain_framebuffers[i]) != VK_SUCCESS)
      throw std::runtime_error("failed to create framebuffer!");
  }
}

//###################################################################
/** Creates uniform buffers between CPU and GPU in a coherent sence.*/
void ChiSim::CreateUniformBuffers()
{
  VkDeviceSize bufferSize = sizeof(UniformBufferObject);

  m_uniform_buffers.resize(m_swap_chain_images.size());
  m_uniform_buffers_memory.resize(m_swap_chain_images.size());

  for (size_t i = 0; i < m_swap_chain_images.size(); i++)
  {
    CreateBuffer(bufferSize,
                 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 m_uniform_buffers[i],
                 m_uniform_buffers_memory[i]);
  }
}

//###################################################################
/** Create descriptor pool. */
void ChiSim::CreateDescriptorPool()
{
  std::array<VkDescriptorPoolSize, 2> poolSizes = {};
  poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSizes[0].descriptorCount = m_swap_chain_images.size();
  poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  poolSizes[1].descriptorCount = m_swap_chain_images.size();

  VkDescriptorPoolCreateInfo poolInfo = {};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = poolSizes.size();
  poolInfo.pPoolSizes = poolSizes.data();
  poolInfo.maxSets = m_swap_chain_images.size();

  if (vkCreateDescriptorPool(m_device,
                             &poolInfo,
                             nullptr,
                             &m_descriptor_pool) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor pool!");
  }
}

//###################################################################
/** Create descriptor sets. */
void ChiSim::CreateDescriptorSets()
{
  size_t num_swap_images = m_swap_chain_images.size();
  std::vector<VkDescriptorSetLayout>
    layouts(num_swap_images, m_descriptor_set_layout);
  VkDescriptorSetAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = m_descriptor_pool;
  allocInfo.descriptorSetCount = num_swap_images;
  allocInfo.pSetLayouts = layouts.data();

  m_descriptor_sets.resize(num_swap_images);
  if (vkAllocateDescriptorSets(m_device,
                               &allocInfo,
                               m_descriptor_sets.data()) != VK_SUCCESS)
    throw std::runtime_error("failed to allocate descriptor sets!");

  for (size_t i = 0; i < num_swap_images; i++) {
    VkDescriptorBufferInfo bufferInfo = {};
    bufferInfo.buffer = m_uniform_buffers[i];
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);

    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = m_texture_image_view;
    imageInfo.sampler = m_texture_sampler;

    std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};
    descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[0].dstSet = m_descriptor_sets[i];
    descriptorWrites[0].dstBinding = 0;
    descriptorWrites[0].dstArrayElement = 0;
    descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrites[0].descriptorCount = 1;
    descriptorWrites[0].pBufferInfo = &bufferInfo;

    descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrites[1].dstSet = m_descriptor_sets[i];
    descriptorWrites[1].dstBinding = 1;
    descriptorWrites[1].dstArrayElement = 0;
    descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrites[1].descriptorCount = 1;
    descriptorWrites[1].pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(m_device,
                           descriptorWrites.size(),
                           descriptorWrites.data(), 0, nullptr);
  }
}