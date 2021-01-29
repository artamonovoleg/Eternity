cmake_minimum_required(VERSION 3.16)
project(Eternity CXX)
set(CMAKE_CXX_STANDARD 17)

add_compile_options(-g)

set(VENDORS ../vendor/glfw/include
            ../vendor/glm
            ../vendor/VulkanHelper/includes
            ../vendor/stb_image)

include_directories(${VENDORS}
                    ./
                    ./API/Vulkan/
                    ./Core
                    ./Events
                    ./Input
                    ./Utils)

add_definitions(-DET_DEBUG)
add_executable(Eternity     main.cpp
                            ./Core/Window.cpp
                            ./Events/EventSystem.cpp
                            ./Input/Input.cpp
                            ./API/Vulkan/Instance.cpp
                            ./API/Vulkan/Surface.cpp
                            ./API/Vulkan/PhysicalDevice.cpp
                            ./API/Vulkan/Device.cpp
                            ./API/Vulkan/Swapchain.cpp)
                            
target_link_libraries(Eternity vulkan glfw glm tinyobjloader stb_image)