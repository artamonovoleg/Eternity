#include <mutex>
#include <GLFW/glfw3.h>
#include "Window.hpp"

namespace Eternity
{
    namespace 
    {
        GLFWwindow*     pWindow = nullptr;
        std::once_flag  glfwInitFlag;
    }

    void    CreateWindow(int width, int height, const std::string& title)
    {
        std::call_once(glfwInitFlag, [&]()
        {
            glfwInit();
            pWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        });
    }

    void    DestroyWindow()
    {
        glfwDestroyWindow(pWindow);
        glfwTerminate();
    }

    bool    WindowShouldClose()
    {
        return glfwWindowShouldClose(pWindow);
    }

    void    PollEvents()
    {
        glfwPollEvents();
    }

    void    GetWindowSize(int& width, int& height)
    {
        glfwGetWindowSize(pWindow, &width, &height);
    }
    
    int     GetWindowWidth()
    {
        int width;
        glfwGetWindowSize(pWindow, &width, nullptr);
        return width;
    }

    int     GetWindowHeight()
    {
        int height;
        glfwGetWindowSize(pWindow, nullptr, &height);
        return height;
    }

} // namespace Eternity