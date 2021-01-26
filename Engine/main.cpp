#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Base.hpp"
#include "Window.hpp"

int main(int, char**) 
{
    ET_INFO("Info example");
    ET_TRACE("Trace example");
    ET_WARN("Warning example");
    ET_ERROR("Error example");

    Eternity::CreateWindow(800, 600, "H");

    while (!Eternity::WindowShouldClose())
    {
        Eternity::PollEvents();
    }
    
    Eternity::DestroyWindow();
    
    return 0;
}