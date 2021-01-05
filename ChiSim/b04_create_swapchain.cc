#include "chi_sim.h"

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

  VkExtent2D extent = GetSurface2DExtent(swapChainSupport.capabilities);

  //======================================== Find number of swap chain images
  // This is normally at least 2 but can be 3
  // for triple buffering.
  uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
  auto max_image_count = swapChainSupport.capabilities.maxImageCount;

  if (max_image_count > 0 && imageCount > max_image_count)
    imageCount = swapChainSupport.capabilities.maxImageCount;

  //======================================== Populate Swapchain creation struct
  VkSwapchainCreateInfoKHR createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = m_main_surface;

  createInfo.minImageCount = imageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  QueueFamilyIndices qf_indices = FindDeviceQueueFamilies(m_physical_device);
  uint32_t queueFamilyIndices[] =
    {qf_indices.graphicsFamily.value(), qf_indices.presentFamily.value()};

  if (qf_indices.graphicsFamily != qf_indices.presentFamily)
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

  //======================================== Create image views for each image
  m_swap_chain_image_views.resize(m_swap_chain_images.size());

  for (uint32_t i = 0; i < m_swap_chain_images.size(); i++)
    m_swap_chain_image_views[i] =
      CreateImageView(m_swap_chain_images[i],
                      m_swap_chain_image_format);
}

//###################################################################
/** Gets the capabilities of the swap chain surface. */
ChiSim::SwapChainSupportDetails
  ChiSim::QuerySwapChainSupport(VkPhysicalDevice device)
{
  SwapChainSupportDetails details;

  //============================== Get device surface capabilities
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device,
                                            m_main_surface,
                                            &details.capabilities);

  //============================== Get device surface formats
  uint32_t formatCount;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device,
                                       m_main_surface,
                                       &formatCount,
                                       nullptr);

  if (formatCount != 0)
  {
    details.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device,
                                         m_main_surface,
                                         &formatCount,
                                         details.formats.data());
  }

  //============================== Get device surface present modes
  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device,
                                            m_main_surface,
                                            &presentModeCount,
                                            nullptr);

  if (presentModeCount != 0)
  {
    details.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device,
                                              m_main_surface,
                                              &presentModeCount,
                                              details.presentModes.data());
  }

  return details;
}

//###################################################################
/** Chooses a swap surface format. */
VkSurfaceFormatKHR ChiSim::ChooseSwapSurfaceFormat(
  const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
  for (const auto& availableFormat : availableFormats)
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
        availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
      return availableFormat;

  return availableFormats[0];
}

//###################################################################
/** Chooses a swap surface format. */
VkPresentModeKHR ChiSim::ChooseSwapPresentMode(
  const std::vector<VkPresentModeKHR>& availablePresentModes)
{
  for (const auto& availablePresentMode : availablePresentModes)
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
      return availablePresentMode;

  return VK_PRESENT_MODE_FIFO_KHR;
}

//###################################################################
/** Gets surface 2D extent. */
VkExtent2D ChiSim::GetSurface2DExtent(
  const VkSurfaceCapabilitiesKHR& capabilities)
{
  if (capabilities.currentExtent.width != UINT32_MAX)
  {
    return capabilities.currentExtent;
  }
  else
  {
    int width, height;
    glfwGetFramebufferSize(m_main_window, &width, &height);

    VkExtent2D actualExtent =
      { static_cast<uint32_t>(width),
        static_cast<uint32_t>(height) };

    actualExtent.width =
      std::max(capabilities.minImageExtent.width,
               std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height =
      std::max(capabilities.minImageExtent.height,
               std::min(capabilities.maxImageExtent.height, actualExtent.height));

    return actualExtent;
  }
}

//###################################################################
/** Creates vulkan image.*/
void ChiSim::CreateImage(uint32_t width,
                         uint32_t height,
                         VkFormat format,
                         VkImageTiling tiling,
                         VkImageUsageFlags usage,
                         VkMemoryPropertyFlags properties,
                         VkImage& image,
                         VkDeviceMemory& imageMemory)
{
  VkImageCreateInfo imageInfo = {};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = width;
  imageInfo.extent.height = height;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.format = format;
  imageInfo.tiling = tiling;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = usage;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateImage(m_device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
    throw std::runtime_error("failed to create image!");
  }

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(m_device, image, &memRequirements);

  VkMemoryAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex =
    FindMemoryType(memRequirements.memoryTypeBits, properties);

  if (vkAllocateMemory(m_device,
                       &allocInfo,
                       nullptr,
                       &imageMemory) != VK_SUCCESS)
    throw std::runtime_error("failed to allocate image memory!");

  vkBindImageMemory(m_device, image, imageMemory, 0);
}