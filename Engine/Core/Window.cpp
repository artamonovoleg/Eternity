#include <mutex>
#include <GLFW/glfw3.h>
#include "Window.hpp"
#include "Base.hpp"

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
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            pWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
            ET_ASSERT(pWindow);
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

    GLFWwindow* GetWindow()
    {
        return pWindow;
    }
} // namespace Eternity