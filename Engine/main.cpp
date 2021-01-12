#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "Window.hpp"
#include "VulkanRenderer.hpp"
#include "EventSystem.hpp"
#include "Input.hpp"

int main(int, char **) 
{
    Eternity::CreateWindow(800, 600, "Eternity");
    Eternity::EventSystem::Init();
    Eternity::Input::Init();
    Eternity::VulkanRenderer renderer;

    renderer.InitVulkan();

    while (!glfwWindowShouldClose(Eternity::GetCurrentWindow()))
    {
        renderer.DrawFrame();
        Eternity::EventSystem::PollEvents();
    }

    renderer.DeinitVulkan();
    Eternity::DestroyWindow();
    return 0; 
}