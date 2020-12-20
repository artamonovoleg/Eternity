//
// Created by artamonovoleg on 20.12.2020.
//

#pragma once
#include <GLFW/glfw3.h>
#include "Window.hpp"

namespace Eternity
{
    class LinuxWindow : public Window
    {
        private:
            GLFWwindow* m_Window    = nullptr;
        public:
            LinuxWindow(const WindowProps& windowProps);
            ~LinuxWindow() override;
    };
}