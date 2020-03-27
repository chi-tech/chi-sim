#ifndef _ChiSim_h
#define _ChiSim_h

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


//###################################################################
/** Main simulation system class. */
class ChiSim
{
public:
  static ChiSim& GetSystemScope();

  /** Deleted copy constructor. */
  ChiSim(const ChiSim&) = delete;

  void Initialize();
  void ExecuteRuntime();
  void Finalize();

  GLFWwindow* m_main_window;
  VkInstance  m_vulkan_instance;

private:
  /** Default constructor privatized. */
  ChiSim() {}

  /** Declared singleton instance of chi_sim. */
  static ChiSim m_instance;

  void CreateVulkanInstance();
  bool CheckVulkanValidationLayerSupport();
};

#endif