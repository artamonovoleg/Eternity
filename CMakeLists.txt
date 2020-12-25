cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 20)
set(PROJECT_NAME Eternity)

project(${PROJECT_NAME})

# GLFW build setup
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

set(GLFW_INCLUDE_DIR vendor/glfw/include)
set(GLM_INCLUDE_DIR vendor/glm/)
set(STB_INCLUDE_DIR vendor/stb_image/)

set(VENDOR ${GLFW_INCLUDE_DIR} ${GLM_INCLUDE_DIR} ${STB_INCLUDE_DIR})
add_subdirectory(vendor)

set(INCLUDES    ${VENDOR}
                Engine/Core
                Engine/Graphics
                Engine/Utils
                Engine/Platform/Vulkan)

set(SRCS    vendor/stb_image/stb_image.cpp
            Sandbox/main.cpp
            Engine/Platform/Vulkan/InstanceBuilder.cpp
            Engine/Platform/Vulkan/VulkanCheck.cpp
            Engine/Platform/Vulkan/PhysicalDeviceSelector.cpp)

include_directories(${INCLUDES})
add_executable(${PROJECT_NAME} ${SRCS})

# Debug options
add_compile_definitions(ET_DEBUG)

if (WIN32)
    target_link_libraries(${PROJECT_NAME} glfw)
elseif(UNIX AND NOT APPLE)
    add_compile_definitions(ET_PLATFORM_LINUX)
    target_link_libraries(${PROJECT_NAME} glfw vulkan X11 pthread dl)
endif()