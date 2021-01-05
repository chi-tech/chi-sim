#include "chi_sim.h"

//###################################################################
/** Create command buffers. */
void ChiSim::CreateCommandBuffers()
{
  m_command_buffers.resize(m_swap_chain_framebuffers.size());

  VkCommandBufferAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = m_command_pool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = (uint32_t) m_command_buffers.size();

  if (vkAllocateCommandBuffers(m_device,
                               &allocInfo,
                               m_command_buffers.data()) != VK_SUCCESS)
    throw std::runtime_error("failed to allocate command buffers!");

  for (size_t i = 0; i < m_command_buffers.size(); i++)
  {
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(m_command_buffers[i], &beginInfo) != VK_SUCCESS)
      throw std::runtime_error("failed to begin recording command buffer!");

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_render_pass;
    renderPassInfo.framebuffer = m_swap_chain_framebuffers[i];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_swap_chain_extent;

    //============================ Clear background and depth-buffer
    std::array<VkClearValue, 2> clearValues = {};
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[1].depthStencil = {1.0f,0};
    renderPassInfo.clearValueCount = clearValues.size();
    renderPassInfo.pClearValues = clearValues.data();

    //============================ Start rendering
    vkCmdBeginRenderPass(m_command_buffers[i],
                         &renderPassInfo,
                         VK_SUBPASS_CONTENTS_INLINE);

    //============================ Bind a Graphical Material
    vkCmdBindPipeline(m_command_buffers[i],
                      VK_PIPELINE_BIND_POINT_GRAPHICS,
                      m_graphics_pipeline);

    vkCmdBindDescriptorSets(m_command_buffers[i],
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_pipeline_layout,
                            0,
                            1,
                            &m_descriptor_sets[i],
                            0,
                            nullptr);

    //============================ Bind geometry information
    VkBuffer vertexBuffers[] = {m_vertex_buffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(m_command_buffers[i],
                           0,
                           1,
                           vertexBuffers,
                           offsets);

    vkCmdBindIndexBuffer(m_command_buffers[i],
                         m_index_buffer,
                         0,
                         VK_INDEX_TYPE_UINT16);

    //============================ Execute draw
    vkCmdDrawIndexed(m_command_buffers[i],
                     indices.size(),
                     1,
                     0,
                     0,
                     0);

    //============================ End rendering pass
    vkCmdEndRenderPass(m_command_buffers[i]);

    if (vkEndCommandBuffer(m_command_buffers[i]) != VK_SUCCESS)
      throw std::runtime_error("failed to record command buffer!");
  }
}

//###################################################################
/** Create synchronization buffers. */
void ChiSim::CreateSyncObjects()
{
  m_image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
  m_render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
  m_in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);
  m_images_in_flight.resize(m_swap_chain_images.size(), VK_NULL_HANDLE);

  VkSemaphoreCreateInfo semaphoreInfo = {};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceInfo = {};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
  {
    if (vkCreateSemaphore(m_device,
                          &semaphoreInfo,
                          nullptr,
                          &m_image_available_semaphores[i]) != VK_SUCCESS ||
        vkCreateSemaphore(m_device,
                          &semaphoreInfo,
                          nullptr,
                          &m_render_finished_semaphores[i]) != VK_SUCCESS ||
        vkCreateFence(m_device,
                      &fenceInfo, nullptr,
                      &m_in_flight_fences[i]) != VK_SUCCESS)
    {
      throw std::runtime_error("failed to create synchronization "
                               "objects for a frame!");
    }
  }//for
}