cmake_minimum_required(VERSION 3.16)
project(Eternity CXX)
set(CMAKE_CXX_STANDARD 17)

add_compile_options(-g)

set(VENDORS ../vendor/glfw/include
            ../vendor/glm
            ../vendor/VulkanHelper/includes)

include_directories(${VENDORS}
                    Core
                    Events
                    Graphics 
                    Input 
                    Utils
                    ./)

add_executable(eternity main.cpp
                        Events/EventSystem.cpp
                        Input/Input.cpp
                        Core/Window.cpp
                        Graphics/Camera.cpp
                        Graphics/VulkanRenderer.cpp)

target_link_libraries(eternity vulkan glfw ${GLFW_LIBRARIES} glm tinyobjloader VulkanHelper)