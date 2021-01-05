#include "chi_sim.h"

//###################################################################
/** Draws the actual frame. */
void ChiSim::DrawFrame()
{
  vkWaitForFences(m_device,
                  1,
                  &m_in_flight_fences[m_current_frame],
                  VK_TRUE,
                  UINT64_MAX);

  uint32_t imageIndex;
  VkResult result = vkAcquireNextImageKHR(
    m_device,
    m_swap_chain,
    UINT64_MAX,
    m_image_available_semaphores[m_current_frame],
    VK_NULL_HANDLE,
    &imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR)
  { recreateSwapChain(); return; }
  else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    throw std::runtime_error("failed to acquire swap chain image!");

  if (m_images_in_flight[imageIndex] != VK_NULL_HANDLE)
    vkWaitForFences(m_device,
                    1,
                    &m_images_in_flight[imageIndex],
                    VK_TRUE,
                    UINT64_MAX);

  m_images_in_flight[imageIndex] = m_in_flight_fences[m_current_frame];

  UpdateUniformBuffer(imageIndex);

  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore waitSemaphores[] =
    {m_image_available_semaphores[m_current_frame]};
  VkPipelineStageFlags waitStages[] =
    {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;

  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &m_command_buffers[imageIndex];

  VkSemaphore signalSemaphores[] =
    {m_render_finished_semaphores[m_current_frame]};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  vkResetFences(m_device, 1, &m_in_flight_fences[m_current_frame]);

  if (vkQueueSubmit(m_graphics_queue,
                    1,
                    &submitInfo,
                    m_in_flight_fences[m_current_frame]) != VK_SUCCESS)
    throw std::runtime_error("failed to submit draw command buffer!");

  VkPresentInfoKHR presentInfo = {};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;

  VkSwapchainKHR swapChains[] = {m_swap_chain};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;

  presentInfo.pImageIndices = &imageIndex;

  result = vkQueuePresentKHR(m_present_queue, &presentInfo);

  if (result == VK_ERROR_OUT_OF_DATE_KHR ||
      result == VK_SUBOPTIMAL_KHR ||
      m_framebuffer_resized)
  {
    m_framebuffer_resized = false;
    recreateSwapChain();
  }
  else if (result != VK_SUCCESS)
    throw std::runtime_error("failed to present swap chain image!");

  m_current_frame = (m_current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}