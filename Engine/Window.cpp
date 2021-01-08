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

    GLFWwindow* GetCurrentWindow()
    {
        return pWindow;
    }
}