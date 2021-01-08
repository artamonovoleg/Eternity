cmake_minimum_required(VERSION 3.16)
project(Eternity CXX)
set(CMAKE_CXX_STANDARD 17)

add_compile_options(-g)

include_directories(vendor/glfw/include
                    vendor/glm)

add_executable(eternity main.cpp Window.cpp VulkanUtils.cpp VulkanBuildInstance.cpp VulkanBuildDevice.cpp)

target_link_libraries(eternity vulkan glfw ${GLFW_LIBRARIES} glm)