cmake_minimum_required(VERSION 3.16)
project(Eternity CXX)
set(CMAKE_CXX_STANDARD 17)

add_compile_options(-g)

include_directories(vendor/glfw/include
                    vendor/glm
                    Core 
                    Events 
                    Input 
                    Utils)

add_executable(eternity VulkanUtils.cpp VulkanBuildInstance.cpp VulkanBuildDevice.cpp VulkanBuildSwapchain.cpp VulkanShader.cpp VulkanBuildPipeline.cpp VulkanVertexBuffer.cpp VulkanImage.cpp 
                        main.cpp
                        Events/EventSystem.cpp
                        Input/Input.cpp
                        Core/Window.cpp
                        Mesh.cpp 
                        Camera.cpp)

target_link_libraries(eternity vulkan glfw ${GLFW_LIBRARIES} glm tinyobjloader)