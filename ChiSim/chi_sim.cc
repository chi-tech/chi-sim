#include "chi_sim.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

//###################################################################
/** Creates the main GLFW window. */
void ChiSim::CreateMainWindow()
{
  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  m_main_window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
  glfwSetWindowUserPointer(m_main_window, this);
  glfwSetFramebufferSizeCallback(m_main_window, FramebufferResizeCallback);
}

//###################################################################
/** Callback function when window gets resized. */
void ChiSim::FramebufferResizeCallback(GLFWwindow* window,
                                       int width,
                                       int height)
{
  auto& app = ChiSim::GetSystemScope();
  app.m_framebuffer_resized = true;
}

//###################################################################
/** Creates the main vulkan instance. */
void ChiSim::CreateVulkanInstance()
{
  if (k_enable_validation_layers && !CheckValidationLayerSupport())
    throw std::runtime_error("validation layers requested, but not available!");

  VkApplicationInfo appInfo = {};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "Hello Triangle";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "No Engine";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;

  auto extensions = GetRequiredExtensions();
  createInfo.enabledExtensionCount = extensions.size();
  createInfo.ppEnabledExtensionNames = extensions.data();

  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
  if (k_enable_validation_layers)
  {
    createInfo.enabledLayerCount = k_validation_layers.size();
    createInfo.ppEnabledLayerNames = k_validation_layers.data();

    PopulateDebugMessengerCreateInfo(debugCreateInfo);
    createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
  }
  else
  {
    createInfo.enabledLayerCount = 0;
    createInfo.pNext = nullptr;
  }

  if (vkCreateInstance(&createInfo, nullptr, &m_vk_instance) != VK_SUCCESS)
    throw std::runtime_error("failed to create m_vk_instance!");
}

//###################################################################
/** Sets up debug messengers. */
void ChiSim::SetupDebugMessenger()
{
  if (!k_enable_validation_layers) return;

  VkDebugUtilsMessengerCreateInfoEXT createInfo;
  PopulateDebugMessengerCreateInfo(createInfo);

  if (CreateDebugUtilsMessengerEXT(m_vk_instance,
                                   &createInfo,
                                   nullptr,
                                   &m_debug_messenger) != VK_SUCCESS)
    throw std::runtime_error("failed to set up debug messenger!");
}

//###################################################################
/** Creates rendering surface for main window. */
void ChiSim::CreateMainWindowSurface()
{
  if (glfwCreateWindowSurface(m_vk_instance,
                              m_main_window,
                              nullptr,
                              &m_main_surface) != VK_SUCCESS)
    throw std::runtime_error("failed to create m_main_window m_main_surface!");
}

//###################################################################
/** Picks a physical device. */
void ChiSim::PickPhysicalDevice()
{
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(m_vk_instance, &deviceCount, nullptr);

  if (deviceCount == 0) {
    throw std::runtime_error("failed to find GPUs with Vulkan support!");
  }

  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(m_vk_instance, &deviceCount, devices.data());

  for (const auto& device : devices) {
    if (IsDeviceSuitable(device)) {
      m_physical_device = device;
      break;
    }
  }

  if (m_physical_device == VK_NULL_HANDLE) {
    throw std::runtime_error("failed to find a suitable GPU!");
  }
}

//###################################################################
/** Creates a logical device. */
void ChiSim::CreateLogicalDevice()
{
  QueueFamilyIndices indices = FindDeviceQueueFamilies(m_physical_device);

  std::set<uint32_t> uniqueQueueFamilies =
    {indices.graphicsFamily.value(), indices.presentFamily.value()};


  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
  float queuePriority = 1.0f;
  for (uint32_t queueFamily : uniqueQueueFamilies)
  {
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  }

  VkPhysicalDeviceFeatures deviceFeatures = {};
  deviceFeatures.samplerAnisotropy = VK_TRUE;

  VkDeviceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

  createInfo.queueCreateInfoCount = queueCreateInfos.size();
  createInfo.pQueueCreateInfos = queueCreateInfos.data();

  createInfo.pEnabledFeatures = &deviceFeatures;

  createInfo.enabledExtensionCount = k_device_extensions.size();
  createInfo.ppEnabledExtensionNames = k_device_extensions.data();

  if (k_enable_validation_layers)
  {
    createInfo.enabledLayerCount = k_validation_layers.size();
    createInfo.ppEnabledLayerNames = k_validation_layers.data();
  }
  else
  {
    createInfo.enabledLayerCount = 0;
  }

  if (vkCreateDevice(m_physical_device,
                     &createInfo,
                     nullptr,
                     &m_device) != VK_SUCCESS)
    throw std::runtime_error("failed to create logical m_device!");

  vkGetDeviceQueue(m_device,
                   indices.graphicsFamily.value(), 0, &m_graphics_queue);
  vkGetDeviceQueue(m_device,
                   indices.presentFamily.value(), 0, &m_present_queue);
}

//###################################################################
/** Creates a swap chain. */
void ChiSim::CreateSwapChain()
{
  SwapChainSupportDetails swapChainSupport =
    QuerySwapChainSupport(m_physical_device);

  VkSurfaceFormatKHR surfaceFormat =
    ChooseSwapSurfaceFormat(swapChainSupport.formats);
  VkPresentModeKHR presentMode =
    ChooseSwapPresentMode(swapChainSupport.presentModes);
  VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
  if (swapChainSupport.capabilities.maxImageCount > 0 &&
      imageCount > swapChainSupport.capabilities.maxImageCount)
  {
    imageCount = swapChainSupport.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = m_main_surface;

  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  QueueFamilyIndices indices = FindDeviceQueueFamilies(m_physical_device);
  uint32_t queueFamilyIndices[] =
    {indices.graphicsFamily.value(), indices.presentFamily.value()};

  if (indices.graphicsFamily != indices.presentFamily)
  {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else
  {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;

  if (vkCreateSwapchainKHR(m_device,
                           &createInfo,
                           nullptr,
                           &m_swap_chain) != VK_SUCCESS)
    throw std::runtime_error("failed to create swap chain!");

  vkGetSwapchainImagesKHR(m_device,
                          m_swap_chain,
                          &imageCount,
                          nullptr);
  m_swap_chain_images.resize(imageCount);
  vkGetSwapchainImagesKHR(m_device,
                          m_swap_chain,
                          &imageCount,
                          m_swap_chain_images.data());

  m_swap_chain_image_format = surfaceFormat.format;
  m_swap_chain_extent = extent;
}

//###################################################################
/** Creates a image views. */
void ChiSim::CreateImageViews()
{
  m_swap_chain_image_views.resize(m_swap_chain_images.size());

  for (uint32_t i = 0; i < m_swap_chain_images.size(); i++)
    m_swap_chain_image_views[i] =
      CreateImageView(m_swap_chain_images[i],
                      m_swap_chain_image_format);
}

//###################################################################
/** Create render pass. */
void ChiSim::CreateRenderPass() {
  VkAttachmentDescription colorAttachment = {};
  colorAttachment.format         = m_swap_chain_image_format;
  colorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorAttachmentRef = {};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;

  VkSubpassDependency dependency = {};
  dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass    = 0;
  dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

  VkRenderPassCreateInfo renderPassInfo = {};
  renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = 1;
  renderPassInfo.pAttachments    = &colorAttachment;
  renderPassInfo.subpassCount    = 1;
  renderPassInfo.pSubpasses      = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies   = &dependency;

  if (vkCreateRenderPass(m_device,
                         &renderPassInfo,
                         nullptr,
                         &m_render_pass) != VK_SUCCESS)
    throw std::runtime_error("failed to create render pass!");
}

//###################################################################
/** Create graphics pipeline. */
void ChiSim::CreateGraphicsPipeline() {
  auto vertShaderCode = ReadFileToBuffer("../shaders/vert.spv");
  auto fragShaderCode = ReadFileToBuffer("../shaders/frag.spv");

  VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
  VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

  VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
  vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = vertShaderModule;
  vertShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
  fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = fragShaderModule;
  fragShaderStageInfo.pName = "main";

  VkPipelineShaderStageCreateInfo shaderStages[] =
    {vertShaderStageInfo, fragShaderStageInfo};

  VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
  vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

  auto bindingDescription    = Vertex::GetBindingDescription();
  auto attributeDescriptions = Vertex::GetAttributeDescriptions();

  vertexInputInfo.vertexBindingDescriptionCount = 1;
  vertexInputInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
  vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
  vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

  VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
  inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  VkViewport viewport = {};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float) m_swap_chain_extent.width;
  viewport.height = (float) m_swap_chain_extent.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  VkRect2D scissor = {};
  scissor.offset = {0, 0};
  scissor.extent = m_swap_chain_extent;

  VkPipelineViewportStateCreateInfo viewportState = {};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.pViewports = &viewport;
  viewportState.scissorCount = 1;
  viewportState.pScissors = &scissor;

  VkPipelineRasterizationStateCreateInfo rasterizer = {};
  rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizer.depthClampEnable = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizer.lineWidth = 1.0f;
  rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterizer.depthBiasEnable = VK_FALSE;

  VkPipelineMultisampleStateCreateInfo multisampling = {};
  multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampling.sampleShadingEnable = VK_FALSE;
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

  VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
  colorBlendAttachment.colorWriteMask =
    VK_COLOR_COMPONENT_R_BIT |
    VK_COLOR_COMPONENT_G_BIT |
    VK_COLOR_COMPONENT_B_BIT |
    VK_COLOR_COMPONENT_A_BIT;
  colorBlendAttachment.blendEnable = VK_FALSE;

  VkPipelineColorBlendStateCreateInfo colorBlending = {};
  colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlending.logicOpEnable = VK_FALSE;
  colorBlending.logicOp = VK_LOGIC_OP_COPY;
  colorBlending.attachmentCount = 1;
  colorBlending.pAttachments = &colorBlendAttachment;
  colorBlending.blendConstants[0] = 0.0f;
  colorBlending.blendConstants[1] = 0.0f;
  colorBlending.blendConstants[2] = 0.0f;
  colorBlending.blendConstants[3] = 0.0f;

  VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 1;
  pipelineLayoutInfo.pSetLayouts = &m_descriptor_set_layout;

  if (vkCreatePipelineLayout(m_device,
                             &pipelineLayoutInfo,
                             nullptr,
                             &m_pipeline_layout) != VK_SUCCESS)
    throw std::runtime_error("failed to create pipeline layout!");



  VkGraphicsPipelineCreateInfo pipelineInfo = {};
  pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineInfo.stageCount = 2;
  pipelineInfo.pStages = shaderStages;
  pipelineInfo.pVertexInputState = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState = &multisampling;
  pipelineInfo.pColorBlendState = &colorBlending;
  pipelineInfo.layout = m_pipeline_layout;
  pipelineInfo.renderPass = m_render_pass;
  pipelineInfo.subpass = 0;
  pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

  if (vkCreateGraphicsPipelines(m_device,
                                VK_NULL_HANDLE,
                                1,
                                &pipelineInfo,
                                nullptr,
                                &m_graphics_pipeline) != VK_SUCCESS)
    throw std::runtime_error("failed to create graphics pipeline!");

  vkDestroyShaderModule(m_device, fragShaderModule, nullptr);
  vkDestroyShaderModule(m_device, vertShaderModule, nullptr);
}

//###################################################################
/** Create frame buffer. */
void ChiSim::CreateFramebuffers()
{
  m_swap_chain_framebuffers.resize(m_swap_chain_image_views.size());

  for (size_t i = 0; i < m_swap_chain_image_views.size(); i++)
  {
    VkImageView attachments[] = { m_swap_chain_image_views[i] };

    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = m_render_pass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = attachments;
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
/** Create command pool. */
void ChiSim::CreateCommandPool()
{
  QueueFamilyIndices queueFamilyIndices =
    FindDeviceQueueFamilies(m_physical_device);

  VkCommandPoolCreateInfo poolInfo = {};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

  if (vkCreateCommandPool(m_device,
                          &poolInfo,
                          nullptr,
                          &m_command_pool) != VK_SUCCESS)
    throw std::runtime_error("failed to create command pool!");

}

void ChiSim::CreateBuffer(VkDeviceSize size,
                          VkBufferUsageFlags usage,
                          VkMemoryPropertyFlags properties,
                          VkBuffer& buffer,
                          VkDeviceMemory& bufferMemory)
{
  VkBufferCreateInfo bufferInfo = {};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(m_device,
                     &bufferInfo,
                     nullptr,
                     &buffer) != VK_SUCCESS)
    throw std::runtime_error("failed to create buffer!");

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(m_device, buffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex =
    FindMemoryType(memRequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(m_device,
                       &allocInfo,
                       nullptr,
                       &bufferMemory) != VK_SUCCESS)
    throw std::runtime_error("failed to allocate buffer memory!");

  vkBindBufferMemory(m_device, buffer, bufferMemory, 0);
}

//###################################################################
/** Copy one buffer to another. */
void ChiSim::CopyBuffer(VkBuffer srcBuffer,
                        VkBuffer dstBuffer,
                        VkDeviceSize size)
{
  VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

  VkBufferCopy copyRegion = {};
  copyRegion.size = size;
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

  EndSingleTimeCommands(commandBuffer);
}

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

    VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(m_command_buffers[i],
                         &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(m_command_buffers[i],
                      VK_PIPELINE_BIND_POINT_GRAPHICS,
                      m_graphics_pipeline);

    VkBuffer vertexBuffers[] = {m_vertex_buffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(m_command_buffers[i], 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(m_command_buffers[i],
                         m_index_buffer, 0, VK_INDEX_TYPE_UINT16);

    vkCmdBindDescriptorSets(m_command_buffers[i],
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_pipeline_layout, 0, 1,
                            &m_descriptor_sets[i], 0, nullptr);

    vkCmdDrawIndexed(m_command_buffers[i],
                     static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

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
  ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
  ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
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
/** Create descriptor pool. */
void ChiSim::CreateDescriptorPool()
{
//  VkDescriptorPoolSize poolSize = {};
//  poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//  poolSize.descriptorCount = m_swap_chain_images.size();

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

//    VkWriteDescriptorSet descriptorWrite = {};
//    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//    descriptorWrite.dstSet = m_descriptor_sets[i];
//    descriptorWrite.dstBinding = 0;
//    descriptorWrite.dstArrayElement = 0;
//    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//    descriptorWrite.descriptorCount = 1;
//    descriptorWrite.pBufferInfo = &bufferInfo;

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
/** Create texture image view. */
void ChiSim::CreateTextureImageView()
{
  m_texture_image_view =
    CreateImageView(m_texture_image, VK_FORMAT_R8G8B8A8_SRGB);
}

//###################################################################
/** Create image view. */
VkImageView ChiSim::CreateImageView(VkImage image, VkFormat format)
{
  VkImageViewCreateInfo viewInfo = {};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = format;
  viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
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

}