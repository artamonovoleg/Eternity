#include "Window.hpp"

namespace Eternity
{
    GLFWwindow* pWindow;
    std::once_flag glfwInitFlag;

    bool CreateWindow(int width, int height, const std::string& title)
    {
        std::call_once(glfwInitFlag, [] () { glfwInit(); });
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        pWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        return (pWindow != nullptr);
    }

    void DestroyWindow()
    {
        glfwDestroyWindow(pWindow);
        glfwTerminate();
    }

    int GetWindowWidth()
    {
        int width;
        glfwGetWindowSize(pWindow, &width, nullptr);
        return width;
    }

    int GetWindowHeight()
    {
        int height;
        glfwGetWindowSize(pWindow, nullptr, &height);
        return height;
    }

    GLFWwindow* GetCurrentWindow()
    {
        return pWindow;
    }
}