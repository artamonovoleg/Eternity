#include "Input.hpp"

namespace Eternity
{
    PressState keys[350] = { PressState::None };
    PressState buttons[8] = { PressState::None };

    void Input::OnKey(const Event& event)
    {
        auto key = static_cast<const KeyEvent&>(event).GetKey();
        keys[key.key] = key.state; 
    }
    
    void Input::OnMouseButton(const Event& event)
    {
        auto button = static_cast<const MouseButtonEvent&>(event).GetButton();
        buttons[button.button] = button.state;
    }

    void Input::Init()
    {
        EventSystem::AddListener(EventType::KeyEvent, OnKey);
        EventSystem::AddListener(EventType::MouseButtonEvent, OnMouseButton);
    }
    
    bool Input::IsPressed(PressState* buff, int code)
    {
        bool res = (buff[code] == PressState::Press);
        if (res)
            buff[code] = PressState::Held;
        return res;
    }

    bool Input::IsReleased(PressState* buff, int code)
    {
        bool res = (buff[code] == PressState::Released);
        if (res)
            buff[code] = PressState::None;
        return res;
    }

    bool Input::IsDown(PressState* buff, int code)
    {
        return (buff[code] == PressState::Press || buff[code] == PressState::Held);
    }

    bool Input::IsKeyPressed(KeyCode key)   { return IsPressed(keys, key); }
    bool Input::IsKeyDown(KeyCode key)      { return IsDown(keys, key); }
    bool Input::IsKeyReleased(KeyCode key)  { return IsReleased(keys, key); }

    bool Input::IsButtonPressed(MouseCode button)   { return IsPressed(buttons, button); }
    bool Input::IsButtonDown(MouseCode button)      { return IsDown(buttons, button); }
    bool Input::IsButtonReleased(MouseCode button)  { return IsReleased(buttons, button); }
}