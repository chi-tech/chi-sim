#include "chi_sim.h"

#include<iostream>
#include<vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <cstring>

//###################################################################
/** Obtains a reference to the singleton. */
ChiSim& ChiSim::GetSystemScope()
{
  return m_instance;
}

//###################################################################
/** Initializes the system. */
void ChiSim::Initialize()
{
  //============================== Initialize glfw library
  if (!glfwInit()) return;

  //==============================Create main window
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  m_main_window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);

  if (!m_main_window)
  {
    glfwTerminate();
    return;
  }

  CreateVulkanInstance();
}

//###################################################################
/** Executes the runtime. */
void ChiSim::ExecuteRuntime()
{
  glm::mat4 matrix;
  glm::vec4 vec;
  auto test = matrix * vec;

  /* Loop until the user closes the window */
  while (!glfwWindowShouldClose(m_main_window))
  {
    /* Render here */
    // glClear(GL_COLOR_BUFFER_BIT);

    /* Swap front and back buffers */
    glfwSwapBuffers(m_main_window);

    /* Poll for and process events */
    glfwPollEvents();
  }
}

//###################################################################
/** Create Vulkan instance. */
void ChiSim::CreateVulkanInstance()
{
  #ifdef NDEBUG
    const bool enableValidationLayers = false;
  #else
    const bool enableValidationLayers = true;
  #endif

  if (enableValidationLayers && !CheckVulkanValidationLayerSupport()) 
  {
        throw std::runtime_error("validation layers requested, "
                                 "but not available!");
  }

  uint32_t extensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, 
                                         &extensionCount, 
                                         nullptr);

  std::cout << extensionCount << " extensions supported" << std::endl;

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

  uint32_t glfwExtensionCount = 0;
  const char** glfwExtensions = 
    glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  std::vector<VkExtensionProperties> extensions(extensionCount);

  vkEnumerateInstanceExtensionProperties(nullptr, 
                                         &extensionCount, 
                                         extensions.data());

  std::cout << "available extensions:" << std::endl;

  for (const auto& extension : extensions) 
    std::cout << "\t" << extension.extensionName << std::endl;

  createInfo.enabledExtensionCount = glfwExtensionCount;
  createInfo.ppEnabledExtensionNames = glfwExtensions;

  createInfo.enabledLayerCount = 0;

  if (vkCreateInstance(&createInfo, 
                       nullptr, 
                       &m_vulkan_instance) != VK_SUCCESS) 
  {
      throw std::runtime_error("failed to create instance!");
  }
}



//###################################################################
/** Check validation layer support. */
bool ChiSim::CheckVulkanValidationLayerSupport()
{
  #ifdef __APPLE__
    const std::vector<const char*> validationLayers = 
      { "MoltenVK" };
  #else
    const std::vector<const char*> validationLayers = 
        { "VK_LAYER_KHRONOS_validation" };
  #endif 

  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  std::cout << "Number of validation layers: " << layerCount << std::endl;

  for (const char* layerName : validationLayers) 
  {
    bool layerFound = false;

    for (const auto& layerProperties : availableLayers) 
    {
      std::cout 
        << "Comparing " 
        << layerProperties.layerName << " and " 
        << layerName
        << std::endl;
      if (strcmp(layerName, layerProperties.layerName) == 0) 
      {
        layerFound = true;
        break;
      }
    }

    if (!layerFound) {
        return false;
    }
  }

  return true;
}

//###################################################################
/** Finalizes the system. */
void ChiSim::Finalize()
{
  vkDestroyInstance(m_vulkan_instance, nullptr);
  glfwDestroyWindow(m_main_window);

  glfwTerminate();
}