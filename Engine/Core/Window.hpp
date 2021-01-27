#pragma once

#include <string>

namespace Eternity
{
    void    CreateWindow(int width, int height, const std::string& title);
    void    DestroyWindow();

    bool    WindowShouldClose();
    void    PollEvents();
    
    int     GetWindowWidth();
    int     GetWindowHeight();
    void    GetWindowSize(int& width, int& height);

    GLFWwindow* GetWindow();
} // namespace Eternity
