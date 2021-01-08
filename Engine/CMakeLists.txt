cmake_minimum_required(VERSION 3.16)
project(Eternity CXX)

add_compile_options(-g)

include_directories(vendor/glfw/include
                    vendor/glm)

add_executable(eternity main.cpp)

target_link_libraries(eternity vulkan glfw ${GLFW_LIBRARIES} glm)