#include "chi_sim.h"

//###################################################################
/** Picks a physical device. */
void ChiSim::PickPhysicalDevice()
{
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(m_vk_instance, &deviceCount, nullptr);

  if (deviceCount == 0)
    throw std::runtime_error("failed to find GPUs with Vulkan support!");

  std::vector<VkPhysicalDevice> devices(deviceCount);
  vkEnumeratePhysicalDevices(m_vk_instance, &deviceCount, devices.data());

  for (const auto& device : devices)
    if (IsDeviceSuitable(device))
      { m_physical_device = device; break; }

  if (m_physical_device == VK_NULL_HANDLE)
    throw std::runtime_error("failed to find a suitable GPU!");

}

//###################################################################
/** Determines if a device is suitable. */
bool ChiSim::IsDeviceSuitable(VkPhysicalDevice device)
{
  QueueFamilyIndices qf_indices = FindDeviceQueueFamilies(device);

  bool extensionsSupported = CheckDeviceExtensionSupport(device);

  bool swapChainAdequate = false;
  if (extensionsSupported)
  {
    SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
    swapChainAdequate = !swapChainSupport.formats.empty() &&
                        !swapChainSupport.presentModes.empty();
  }

  VkPhysicalDeviceFeatures supportedFeatures;
  vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

  return qf_indices.isComplete() &&
         extensionsSupported &&
         swapChainAdequate &&
         supportedFeatures.samplerAnisotropy;
}

//###################################################################
/** Check device extension support. */
bool ChiSim::CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
  uint32_t extensionCount;
  vkEnumerateDeviceExtensionProperties(device,
                                       nullptr,
                                       &extensionCount,
                                       nullptr);

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(device,
                                       nullptr,
                                       &extensionCount,
                                       availableExtensions.data());

  std::set<std::string> requiredExtensions(k_device_extensions.begin(),
                                           k_device_extensions.end());

  for (const auto& extension : availableExtensions)
    requiredExtensions.erase(extension.extensionName);

  return requiredExtensions.empty();
}

//###################################################################
/** Creates a logical device. */
void ChiSim::CreateLogicalDevice()
{
  QueueFamilyIndices qf_indices = FindDeviceQueueFamilies(m_physical_device);

  std::set<uint32_t> uniqueQueueFamilies =
    {qf_indices.graphicsFamily.value(), qf_indices.presentFamily.value()};


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
                   qf_indices.graphicsFamily.value(), 0, &m_graphics_queue);
  vkGetDeviceQueue(m_device,
                   qf_indices.presentFamily.value(), 0, &m_present_queue);
}

//###################################################################
/** Finds device queue families. */
ChiSim::QueueFamilyIndices ChiSim::FindDeviceQueueFamilies(VkPhysicalDevice device)
{
  QueueFamilyIndices qf_indices;

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device,
                                           &queueFamilyCount,
                                           queueFamilies.data());

  int i = 0;
  for (const auto& queueFamily : queueFamilies)
  {
    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
      qf_indices.graphicsFamily = i;

    VkBool32 presentSupport = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device,
                                         i,
                                         m_main_surface,
                                         &presentSupport);

    if (presentSupport) qf_indices.presentFamily = i;
    if (qf_indices.isComplete()) break;

    ++i;
  }

  return qf_indices;
}