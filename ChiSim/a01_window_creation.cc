#include "chi_sim.h"

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
/** Creates rendering surface for main window. */
void ChiSim::CreateMainWindowSurface()
{
  if (glfwCreateWindowSurface(m_vk_instance,
                              m_main_window,
                              nullptr,
                              &m_main_surface) != VK_SUCCESS)
    throw std::runtime_error("failed to create m_main_window m_main_surface!");
}