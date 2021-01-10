#pragma once

#include "KeyEvent.hpp"
#include "EventSystem.hpp"
#include "KeyCodes.hpp"
#include "MouseCodes.hpp"

namespace Eternity
{
    class Input 
    {
        private:
            static void OnKey(const Event& event);
            static void OnMouseButton(const Event& event);

            static bool IsPressed(PressState* buff, int code);
            static bool IsReleased(PressState* buff, int code);
            static bool IsDown(PressState* buff, int code);
        public:
            static void Init();
            
            static bool IsKeyPressed(KeyCode key);
            static bool IsKeyDown(KeyCode key);
            static bool IsKeyReleased(KeyCode key);

            static bool IsButtonPressed(MouseCode button);
            static bool IsButtonDown(MouseCode button);
            static bool IsButtonReleased(MouseCode button);
    };
}
