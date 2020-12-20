//
// Created by artamonovoleg on 20.12.2020.
//

#include "LinuxWindow.hpp"

namespace Eternity
{
    LinuxWindow::LinuxWindow(const WindowProps &windowProps)
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        GLFWwindow* window = glfwCreateWindow(windowProps.width, windowProps.height, windowProps.name.c_str(), nullptr, nullptr);
        glfwMakeContextCurrent(window);
    }

    LinuxWindow::~LinuxWindow()
    {
        glfwDestroyWindow(m_Window);
        glfwTerminate();
    }
}