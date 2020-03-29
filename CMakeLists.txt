cmake_minimum_required(VERSION 3.2)

set(TARGET app2)
project(${TARGET} C CXX)

include_directories("/usr/local/include")

link_directories("/usr/local/lib")

set(LIBS glfw.3.3 vulkan.1 vulkan.1.2.131)

set(SOURCES "main.cc" )
set(SOURCES ${SOURCES} ChiSim/chi_sim.cc)
set(SOURCES ${SOURCES} ChiSim/vulkan_utilities.cc)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${TARGET} ${SOURCES})
target_link_libraries(${TARGET} ${LIBS})
