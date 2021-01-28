#pragma once

#include <string>
#include <functional>

namespace Eternity
{
    void    CreateWindow(int width, int height, const std::string& title);
    void    DestroyWindow();

    bool    WindowShouldClose();
    
    int     GetWindowWidth();
    int     GetWindowHeight();
    void    GetWindowSize(int& width, int& height);

    
    GLFWwindow* GetWindow();
} // namespace Eternity
