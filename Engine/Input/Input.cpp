#include "Input.hpp"
#include "Event.hpp"
#include "EventSystem.hpp"
#include "Window.hpp"

namespace Eternity
{
    PressState keys[350] = { PressState::None };
    PressState buttons[7] = { PressState::None };
    float       mouseScrollDelta = 0.0f;

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

    bool        Input::GetKey(uint16_t key) { return Get(key, keys); }
    bool        Input::GetKeyDown(uint16_t key) { return GetDown(key, keys); }
    bool        Input::GetKeyUp(uint16_t key) { return GetUp(key, keys); }

    bool        Input::GetButton(uint16_t button) { return Get(button, buttons); }
    bool        Input::GetButtonDown(uint16_t button) { return GetDown(button, buttons); }
    bool        Input::GetButtonUp(uint16_t button) { return GetUp(button, buttons); }

    glm::vec3   Input::GetMousePosition() 
    { 
        double xpos;
        double ypos;
        glfwGetCursorPos(Eternity::GetWindow(), &xpos, &ypos);
        return glm::vec3((float)xpos, (float)ypos, 0); 
    }

    float       Input::GetMouseScrollDelta() { return mouseScrollDelta; }

    void        Input::SetMouseMode(MouseMode mode)
    {
        switch(mode)
        {
            case MouseMode::Capture:
                glfwSetInputMode(Eternity::GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                break;
            case MouseMode::Free:
                glfwSetInputMode(Eternity::GetWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                break;
        }
    }

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

        EventSystem::AddListener(EventType::MouseScrollEvent, [](const Event& event)
        {
            auto mouseScrollEvent = static_cast<const MouseScrollEvent&>(event);
            mouseScrollDelta = mouseScrollEvent.GetOffset();
        });
    }
} // namespace Eternity