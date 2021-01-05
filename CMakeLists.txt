cmake_minimum_required(VERSION 3.2)

#------------------------------------------------ DEPENDENCIES
if (NOT DEFINED STB_ROOT)
    if (NOT (DEFINED ENV{STB_ROOT}))
        message(FATAL_ERROR "***** STB_ROOT is not set *****")
    else()
        set(STB_ROOT "$ENV{STB_ROOT}")
    endif()
endif()
message(STATUS "STB_ROOT set to ${STB_ROOT}")

if (NOT DEFINED GLFW_ROOT)
    if (NOT (DEFINED ENV{GLFW_ROOT}))
        message(FATAL_ERROR "***** GLFW_ROOT is not set *****")
    else()
        set(GLFW_ROOT "$ENV{GLFW_ROOT}")
    endif()
endif()
message(STATUS "GLFW_ROOT set to ${GLFW_ROOT}")

if (NOT DEFINED GLM_ROOT)
    if (NOT (DEFINED ENV{GLM_ROOT}))
        message(FATAL_ERROR "***** GLM_ROOT is not set *****")
    else()
        set(GLM_ROOT "$ENV{GLM_ROOT}")
    endif()
endif()
message(STATUS "GLM_ROOT set to ${GLM_ROOT}")

if (NOT DEFINED VK_SDK_PATH)
    if (NOT (DEFINED ENV{VK_SDK_PATH}))
        message(FATAL_ERROR "***** VK_SDK_PATH is not set *****")
    else()
        set(VK_SDK_PATH "$ENV{VK_SDK_PATH}")
    endif()
endif()
message(STATUS "VK_SDK_PATH set to ${VK_SDK_PATH}")

#------------------------------------------------ INCLUDES AND LIBS
set(TARGET app2)
project(${TARGET} C CXX)

include_directories("${STB_ROOT}")
include_directories("${GLFW_ROOT}/include")
include_directories("${GLM_ROOT}")
include_directories("${VK_SDK_PATH}/include")

link_directories("${GLFW_ROOT}/lib")
link_directories("${VK_SDK_PATH}/lib")

set(LIBS glfw3 vulkan-1)

set(SOURCES "main.cc" )
set(SOURCES ${SOURCES} ChiSim/chi_sim.cc)
set(SOURCES ${SOURCES} ChiSim/vulkan_utilities.cc)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${TARGET} ${SOURCES})
target_link_libraries(${TARGET} ${LIBS})
