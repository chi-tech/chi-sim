#include "chi_sim.h"

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
/** Check that validation layers are supported.*/
bool ChiSim::CheckValidationLayerSupport()
{
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  for (const char* layerName : k_validation_layers)
  {
    bool layerFound = false;

    for (const auto& layerProperties : availableLayers)
      if (strcmp(layerName, layerProperties.layerName) == 0)
        { layerFound = true; break; }

    if (!layerFound) return false;
  }

  return true;
}

//###################################################################
/** Gets the window system's required extensions. */
std::vector<const char*> ChiSim::GetRequiredExtensions()
{
  uint32_t glfwExtensionCount = 0;
  const char** glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector<const char*> extensions(glfwExtensions, glfwExtensions +
                                                      glfwExtensionCount);

  if (k_enable_validation_layers)
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

  return extensions;
}