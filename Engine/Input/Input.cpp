#include "Input.hpp"
#include "Event.hpp"
#include "EventSystem.hpp"

namespace Eternity
{
    PressState keys[350] = { PressState::None };
    PressState buttons[7] = { PressState::None };

    bool Input::Get(uint16_t code, PressState* buff)
    {
        return (buff[code] == PressState::Held || buff[code] == PressState::Pressed);
    }

    bool Input::GetDown(uint16_t code, PressState* buff)
    {
        bool ret = (buff[code] == PressState::Pressed);
        buff[code] = PressState::Held;
        return ret;
    }

    bool Input::GetUp(uint16_t code, PressState* buff)
    {
        bool ret = (buff[code] == PressState::Released);
        buff[code] = PressState::None;
        return ret;
    }

    bool Input::GetKey(uint16_t key) { return Get(key, keys); }
    bool Input::GetKeyDown(uint16_t key) { return GetDown(key, keys); }
    bool Input::GetKeyUp(uint16_t key) { return GetUp(key, keys); }

    void Input::Init()
    {
        EventSystem::AddListener(EventType::KeyEvent, [](const Event& event)
        {
            auto keyEvent = static_cast<const KeyEvent&>(event);
            keys[keyEvent.GetKey().key] = keyEvent.GetKey().state;
        });

        EventSystem::AddListener(EventType::MouseEvent, [](const Event& event)
        {
            auto mouseEvent = static_cast<const MouseEvent&>(event);
            buttons[mouseEvent.GetButton().button] = mouseEvent.GetButton().state;
        });
    }
} // namespace Eternity