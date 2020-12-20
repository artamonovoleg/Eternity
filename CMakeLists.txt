cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(PROJECT_NAME Eternity)

project(${PROJECT_NAME})

add_compile_options(
        -g
        -Wall
)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

set(GLFW_INCLUDE_DIR vendor/glfw/include)

add_subdirectory(vendor)

set(INCLUDES    vendor/glfw/include
                vendor/glm/
                vendor/stb_image/
                ./
                Engine/Core
                Engine/Graphics
                Engine/Utils
                Engine/Platform/Vulkan)

set(SRCS    vendor/stb_image/stb_image.cpp
            main.cpp
            Engine/Core/Window.cpp
            Engine/Platform/Linux/LinuxWindow.cpp
            Engine/Graphics/GraphicsContext.cpp
            Engine/Graphics/Renderer.cpp
            Engine/Graphics/RendererCommand.cpp
            Engine/Graphics/RendererAPI.cpp
            Engine/Platform/Vulkan/VkRendererAPI.cpp
            Engine/Platform/Vulkan/VkDebugHelper.cpp)

include_directories(${INCLUDES})

add_executable(${PROJECT_NAME} ${SRCS})

add_definitions(
                -DET_DEBUG
#               -DET_ENABLE_VULKAN_DEBUG
#               -DET_RUNTIME_VULKAN_DEBUG
                )

link_directories(${INCLUDES}
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glfw/src
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/spdlog/includes)

if    (WIN32)
    target_link_libraries(${PROJECT_NAME} glfw glu32 opengl32)
elseif(APPLE)
    find_library(COCOA_Framework Cocoa)
    find_library(OpenGL_Framework OpenGL)
    find_library(IOKit_Framework IOKit)
    find_library(CoreVideo_Framework CoreVideo)

    target_link_libraries(${PROJECT_NAME}
            glfw
            ${COCOA_Framework} ${OpenGL_Framework}
            ${IOKit_Framework} ${CoreVideo_Framework})
elseif(UNIX AND NOT APPLE)
    add_definitions(-DET_PLATFORM_LINUX -DET_DEBUG)
    target_link_libraries(${PROJECT_NAME}
            glfw
            vulkan X11 pthread dl)
endif()