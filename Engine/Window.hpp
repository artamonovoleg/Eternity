#pragma once
#include <mutex>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Eternity
{
    bool CreateWindow(int width, int height, const std::string& title);
    void DestroyWindow();

    GLFWwindow* GetCurrentWindow();
}