//
// Created by artamonovoleg on 20.12.2020.
//

#include "Window.hpp"

namespace Eternity
{
    std::unique_ptr<Window> Window::Create(const WindowProps& windowProps)
    {
        return std::unique_ptr<Window>();
    }
}