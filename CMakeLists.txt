cmake_minimum_required(VERSION 3.2)

set(TARGET app2)
project(${TARGET} C CXX)

set(VULKAN_ROOT "/Users/janv4/Desktop/Projects/Vulkan/vulkan-1.2.131.2/macOS")
set(MOLTENVK_ROOT "/Users/janv4/Desktop/Projects/Vulkan/vulkan-1.2.131.2/MoltenVK")
set(GLFW_ROOT "/usr/local/Cellar/glfw/3.3.2")
set(GLM_ROOT "/usr/local/Cellar/glm/0.9.9.5")

# include_directories("${VULKAN_ROOT}/include")
include_directories("${MOLTENVK_ROOT}/include")
include_directories("${GLFW_ROOT}/include")
include_directories("${GLM_ROOT}/include")

# link_directories("${VULKAN_ROOT}/lib")
link_directories("${MOLTENVK_ROOT}/macOS/dynamic/")
link_directories("${GLFW_ROOT}/lib")
link_directories("${GLM_ROOT}/lib")

# set(LIBS glfw.3.3 vulkan.1 vulkan.1.2.131)
set(LIBS glfw.3.3 MoltenVK)

set(SOURCES "main.cc" )
set(SOURCES ${SOURCES} ChiSim/chi_sim.cc)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${TARGET} ${SOURCES})
target_link_libraries(${TARGET} ${LIBS})
