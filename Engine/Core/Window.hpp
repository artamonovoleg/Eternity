//
// Created by artamonovoleg on 20.12.2020.
//

#pragma once
#include <memory>
#include <string>

namespace Eternity
{
    struct WindowProps
    {
        int width;
        int height;
        std::string name;
    };

    class Window
    {
        private:
        public:
            Window()            = default;
            virtual ~Window()   = default;
            static std::unique_ptr<Window> Create(const WindowProps& windowProps);
    };
}