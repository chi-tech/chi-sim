#include "chi_sim.h"

//###################################################################
/** Creates vertex buffers. */
void ChiSim::CreateVertexBuffer()
{
  VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  CreateBuffer(bufferSize,
               VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               stagingBuffer,
               stagingBufferMemory);

  void* data;
  vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, vertices.data(), (size_t) bufferSize);
  vkUnmapMemory(m_device, stagingBufferMemory);

  CreateBuffer(bufferSize,
               VK_BUFFER_USAGE_TRANSFER_DST_BIT |
               VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
               m_vertex_buffer,
               m_vertex_buffer_memory);

  CopyBuffer(stagingBuffer, m_vertex_buffer, bufferSize);

  vkDestroyBuffer(m_device, stagingBuffer, nullptr);
  vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}

//###################################################################
/** Create index buffers. */
void ChiSim::CreateIndexBuffer()
{
  VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  CreateBuffer(bufferSize,
               VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
               stagingBuffer,
               stagingBufferMemory);

  void* data;
  vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, indices.data(), (size_t) bufferSize);
  vkUnmapMemory(m_device, stagingBufferMemory);

  CreateBuffer(bufferSize,
               VK_BUFFER_USAGE_TRANSFER_DST_BIT |
               VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
               m_index_buffer,
               m_index_buffer_memory);

  CopyBuffer(stagingBuffer, m_index_buffer, bufferSize);

  vkDestroyBuffer(m_device, stagingBuffer, nullptr);
  vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}



//###################################################################
/** Update uniform buffer. */
void ChiSim::UpdateUniformBuffer(uint32_t currentImage)
{
  static auto startTime = std::chrono::high_resolution_clock::now();

  auto currentTime = std::chrono::high_resolution_clock::now();
  float time =
    std::chrono::duration<float, std::chrono::seconds::period>(
      currentTime - startTime).count();

  UniformBufferObject ubo = {};
  ubo.model = glm::rotate(glm::mat4(1.0f),
                          time * glm::radians(90.0f),
                          glm::vec3(0.0f, 0.0f, 1.0f));
  ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f),
                         glm::vec3(0.0f, 0.0f, 0.0f),
                         glm::vec3(0.0f, 0.0f, 1.0f));
  ubo.proj = glm::perspective(glm::radians(45.0f),
                              m_swap_chain_extent.width /
                              (float) m_swap_chain_extent.height, 0.1f, 10.0f);
  ubo.proj[1][1] *= -1;

  void* data;
  vkMapMemory(m_device,
              m_uniform_buffers_memory[currentImage],
              0, sizeof(ubo), 0, &data);
  memcpy(data, &ubo, sizeof(ubo));
  vkUnmapMemory(m_device, m_uniform_buffers_memory[currentImage]);
}

//###################################################################
/** Reads a file to a buffer. */
std::vector<char> ChiSim::ReadFileToBuffer(const std::string& filename)
{
  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  if (!file.is_open())
    throw std::runtime_error("failed to open file!");

  size_t fileSize = (size_t) file.tellg();
  std::vector<char> buffer(fileSize);

  file.seekg(0);
  file.read(buffer.data(), fileSize);

  file.close();

  return buffer;
}