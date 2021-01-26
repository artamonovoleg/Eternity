cmake_minimum_required(VERSION 3.16)
project(Eternity CXX)
set(CMAKE_CXX_STANDARD 17)

add_compile_options(-g)

set(VENDORS ../vendor/glfw/include
            ../vendor/glm
            ../vendor/VulkanHelper/includes
            ../vendor/stb_image)

include_directories(${VENDORS}
                    ./)

add_executable(Eternity main.cpp)

target_link_libraries(Eternity vulkan glfw glm tinyobjloader stb_image)