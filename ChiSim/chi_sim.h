#ifndef _ChiSim_h
#define _ChiSim_h

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>



#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <optional>
#include <set>
#include <array>
#include <chrono>

//###################################################################
/** Main simulation system class. */
class ChiSim
{
public:
  const int WIDTH = 800;
  const int HEIGHT = 600;

  const int MAX_FRAMES_IN_FLIGHT = 2;

  const std::vector<const char*> k_validation_layers =
    {"VK_LAYER_KHRONOS_validation"};

  const std::vector<const char*> k_device_extensions =
    {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

  struct Vertex
  {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription GetBindingDescription()
    {
      VkVertexInputBindingDescription bindingDescription = {};
      bindingDescription.binding = 0;
      bindingDescription.stride = sizeof(Vertex);
      bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

      return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3>
      GetAttributeDescriptions()
    {
      std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};

      attributeDescriptions[0].binding = 0;
      attributeDescriptions[0].location = 0;
      attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
      attributeDescriptions[0].offset = offsetof(Vertex, pos);

      attributeDescriptions[1].binding = 0;
      attributeDescriptions[1].location = 1;
      attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
      attributeDescriptions[1].offset = offsetof(Vertex, color);

      attributeDescriptions[2].binding = 0;
      attributeDescriptions[2].location = 2;
      attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
      attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

      return attributeDescriptions;
    }
  };

  const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
  };

  const std::vector<uint16_t> indices = {
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4
  };

  struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
  };

#ifdef NDEBUG
  const bool k_enable_validation_layers = false;
#else
  const bool k_enable_validation_layers = true;
#endif

  GLFWwindow*                    m_main_window;

  VkInstance                     m_vk_instance;
  VkDebugUtilsMessengerEXT       m_debug_messenger;
  VkSurfaceKHR                   m_main_surface;

  VkPhysicalDevice               m_physical_device = VK_NULL_HANDLE;
  VkDevice                       m_device;

  VkQueue                        m_graphics_queue;
  VkQueue                        m_present_queue;

  VkSwapchainKHR                 m_swap_chain;
  std::vector<VkImage>           m_swap_chain_images;
  VkFormat                       m_swap_chain_image_format;
  VkExtent2D                     m_swap_chain_extent;
  std::vector<VkImageView>       m_swap_chain_image_views;
  std::vector<VkFramebuffer>     m_swap_chain_framebuffers;

  VkRenderPass                   m_render_pass;
  VkDescriptorSetLayout          m_descriptor_set_layout;
  VkPipelineLayout               m_pipeline_layout;
  VkPipeline                     m_graphics_pipeline;

  VkCommandPool                  m_command_pool;
  std::vector<VkCommandBuffer>   m_command_buffers;

  std::vector<VkSemaphore>       m_image_available_semaphores;
  std::vector<VkSemaphore>       m_render_finished_semaphores;
  std::vector<VkFence>           m_in_flight_fences;
  std::vector<VkFence>           m_images_in_flight;
  size_t                         m_current_frame = 0;

  bool                           m_framebuffer_resized = false;

  VkBuffer                       m_vertex_buffer;
  VkDeviceMemory                 m_vertex_buffer_memory;

  VkBuffer                       m_index_buffer;
  VkDeviceMemory                 m_index_buffer_memory;

  std::vector<VkBuffer>          m_uniform_buffers;
  std::vector<VkDeviceMemory>    m_uniform_buffers_memory;

  VkDescriptorPool               m_descriptor_pool;

  std::vector<VkDescriptorSet>   m_descriptor_sets;

  VkImage                        m_texture_image;
  VkDeviceMemory                 m_texture_image_memory;

  VkImageView                    m_texture_image_view;
  VkSampler                      m_texture_sampler;

  VkImage                        m_depth_image;
  VkDeviceMemory                 m_depth_image_memory;
  VkImageView                    m_depth_image_view;

  /** Obtains a reference to the singleton instance. */
  static ChiSim& GetSystemScope()
    { return m_instance; }

  /** Deleted copy constructor. */
  ChiSim(const ChiSim&) = delete;

  void Execute() {
    CreateMainWindow();
    InitializeVulkan();
    mainLoop();
    cleanup();
  }



private:
  static VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger);

  static void DestroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator);

  struct QueueFamilyIndices
  {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
      return graphicsFamily.has_value() && presentFamily.has_value();
    }
  };

  struct SwapChainSupportDetails
  {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
  };

  /** Default constructor privatized. */
  ChiSim() {}

  /** Declared singleton m_vk_instance of chi_sim. */
  static ChiSim m_instance;

  void CreateMainWindow();

  static void FramebufferResizeCallback(GLFWwindow* window, int width,
                                                            int height);

  void InitializeVulkan() {
    CreateVulkanInstance();
    SetupDebugMessenger();
    CreateMainWindowSurface();
    PickPhysicalDevice();
    CreateLogicalDevice();
    CreateSwapChain();
    CreateImageViews();
    CreateRenderPass();
    CreateDescriptorSetLayout();
    CreateGraphicsPipeline();
    CreateFramebuffers();
    CreateCommandPool();
    CreateDepthResources();
    CreateTextureImage();
    CreateTextureImageView();
    CreateTextureSampler();
    CreateVertexBuffer();
    CreateIndexBuffer();
    CreateUniformBuffers();
    CreateDescriptorPool();
    CreateDescriptorSets();
    CreateCommandBuffers();
    CreateSyncObjects();
  }

  void mainLoop()
  {
    while (!glfwWindowShouldClose(m_main_window))
    {
      glfwPollEvents();
      DrawFrame();
    }

    vkDeviceWaitIdle(m_device);
  }

  void cleanupSwapChain() {
    for (auto framebuffer : m_swap_chain_framebuffers) {
      vkDestroyFramebuffer(m_device, framebuffer, nullptr);
    }

    vkFreeCommandBuffers(m_device, m_command_pool, static_cast<uint32_t>(m_command_buffers.size()), m_command_buffers.data());

    vkDestroyPipeline(m_device, m_graphics_pipeline, nullptr);
    vkDestroyPipelineLayout(m_device, m_pipeline_layout, nullptr);
    vkDestroyRenderPass(m_device, m_render_pass, nullptr);

    for (auto imageView : m_swap_chain_image_views) {
      vkDestroyImageView(m_device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(m_device, m_swap_chain, nullptr);

    for (size_t i = 0; i < m_swap_chain_images.size(); i++) {
      vkDestroyBuffer(m_device, m_uniform_buffers[i], nullptr);
      vkFreeMemory(m_device, m_uniform_buffers_memory[i], nullptr);
    }

    vkDestroyDescriptorPool(m_device, m_descriptor_pool, nullptr);
  }

  void cleanup() {
    cleanupSwapChain();

    vkDestroySampler(m_device, m_texture_sampler, nullptr);
    vkDestroyImageView(m_device, m_texture_image_view, nullptr);

    vkDestroyImage(m_device, m_texture_image, nullptr);
    vkFreeMemory(m_device, m_texture_image_memory, nullptr);

    vkDestroyDescriptorSetLayout(m_device, m_descriptor_set_layout, nullptr);

    vkDestroyBuffer(m_device, m_vertex_buffer, nullptr);
    vkFreeMemory(m_device, m_vertex_buffer_memory, nullptr);

    vkDestroyBuffer(m_device, m_index_buffer, nullptr);
    vkFreeMemory(m_device, m_index_buffer_memory, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      vkDestroySemaphore(m_device, m_render_finished_semaphores[i], nullptr);
      vkDestroySemaphore(m_device, m_image_available_semaphores[i], nullptr);
      vkDestroyFence(m_device, m_in_flight_fences[i], nullptr);
    }

    vkDestroyCommandPool(m_device, m_command_pool, nullptr);

    vkDestroyDevice(m_device, nullptr);

    if (k_enable_validation_layers) {
      DestroyDebugUtilsMessengerEXT(m_vk_instance, m_debug_messenger, nullptr);
    }

    vkDestroySurfaceKHR(m_vk_instance, m_main_surface, nullptr);
    vkDestroyInstance(m_vk_instance, nullptr);

    glfwDestroyWindow(m_main_window);

    glfwTerminate();
  }

  void recreateSwapChain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(m_main_window, &width, &height);
    while (width == 0 || height == 0) {
      glfwGetFramebufferSize(m_main_window, &width, &height);
      glfwWaitEvents();
    }

    vkDeviceWaitIdle(m_device);

    cleanupSwapChain();

    CreateSwapChain();
    CreateImageViews();
    CreateRenderPass();
    CreateGraphicsPipeline();
    CreateFramebuffers();
    CreateUniformBuffers();
    CreateDescriptorPool();
    CreateDescriptorSets();
    CreateCommandBuffers();
  }

  void CreateVulkanInstance();

  void PopulateDebugMessengerCreateInfo(
    VkDebugUtilsMessengerCreateInfoEXT& createInfo);

  void SetupDebugMessenger();
  void CreateMainWindowSurface();
  void PickPhysicalDevice();
  void CreateLogicalDevice();
  void CreateSwapChain();
  void CreateImageViews();
  void CreateRenderPass();
  void CreateGraphicsPipeline();
  void CreateFramebuffers();
  void CreateCommandPool();
  void CreateBuffer(VkDeviceSize size,
                    VkBufferUsageFlags usage,
                    VkMemoryPropertyFlags properties,
                    VkBuffer& buffer,
                    VkDeviceMemory& bufferMemory);
  void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
  void CreateVertexBuffer();
  void CreateIndexBuffer();
  void CreateUniformBuffers();
  void CreateCommandBuffers();
  void CreateSyncObjects();
  void DrawFrame()
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

  VkShaderModule CreateShaderModule(const std::vector<char>& code);

  VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

  VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

  VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

  SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);

  bool IsDeviceSuitable(VkPhysicalDevice device);

  bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

  QueueFamilyIndices FindDeviceQueueFamilies(VkPhysicalDevice device);

  std::vector<const char*> GetRequiredExtensions();

  bool CheckValidationLayerSupport();

  static std::vector<char> ReadFileToBuffer(const std::string& filename);

  static VKAPI_ATTR VkBool32 VKAPI_CALL
    DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                  VkDebugUtilsMessageTypeFlagsEXT messageType,
                  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                  void* pUserData);

  uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

  void CreateDescriptorSetLayout();

  void UpdateUniformBuffer(uint32_t currentImage);

  void CreateDescriptorPool();
  void CreateDescriptorSets();

  void CreateTextureImage();

  void CreateImage(uint32_t width,
                   uint32_t height,
                   VkFormat format,
                   VkImageTiling tiling,
                   VkImageUsageFlags usage,
                   VkMemoryPropertyFlags properties,
                   VkImage& image,
                   VkDeviceMemory& imageMemory);

  VkCommandBuffer BeginSingleTimeCommands();
  void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

  void TransitionImageLayout(VkImage image,
                             VkFormat format,
                             VkImageLayout oldLayout,
                             VkImageLayout newLayout);

  void CopyBufferToImage(VkBuffer buffer,
                         VkImage image,
                         uint32_t width,
                         uint32_t height);

  void CreateTextureImageView();
  VkImageView CreateImageView(VkImage image, VkFormat format);

  void CreateTextureSampler();

  void CreateDepthResources();
};

#endif