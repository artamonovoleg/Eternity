#pragma once

#include "Base.hpp"

enum class EventType
{
    None,
    WindowResizeEvent,
    KeyEvent,
    MouseEvent,
    MouseScrollEvent,
    MouseMoveEvent,
};

class Event
{
    public:
        virtual ~Event() = default;
        virtual EventType GetType() const { return EventType::None; };
};

class WindowResizeEvent : public Event
{
    private:
        struct WindowSize
        {
            int width;
            int height;
        };

        WindowSize m_Size;
    public:
        WindowResizeEvent(int width, int height)
            : m_Size({ width, height }) 
        {
            ET_INFO("[ Window resize event ] Size:", m_Size.width, m_Size.height);
        }
        
        ~WindowResizeEvent() override = default;

        const WindowSize& GetSize() const { return m_Size; }

        EventType GetType() const override { return EventType::WindowResizeEvent; }
};

enum class PressState
{
    None,
    Released,
    Pressed,
    Held
};

class KeyEvent : public Event
{
    private:
        struct Key
        {
            int         key;
            PressState  state;
        };

        Key m_Key;
    public:
        KeyEvent(int key, int action)
            : m_Key({ key, static_cast<PressState>(action + 1) }) 
        {
            ET_INFO("[ Key event ] Key:", m_Key.key, "State: ", (int)m_Key.state);
        }

        ~KeyEvent() override = default;

        const Key& GetKey() const { return m_Key; };

        EventType GetType() const override { return EventType::KeyEvent; }
};

class MouseEvent : public Event
{
    private:
        struct Button
        {
            int         button;
            PressState  state;
        };

        Button m_Button;
    public:
        MouseEvent(int button, int action)
            : m_Button({ button, static_cast<PressState>(action + 1) }) 
        {
            ET_INFO("[ Mouse event ] Key:", m_Button.button, "State: ", (int)m_Button.state);
        }

        ~MouseEvent() override = default;

        const Button& GetButton() const { return m_Button; };

        EventType GetType() const override { return EventType::MouseEvent; }
};

class MouseMoveEvent : public Event
{
    public:
        MouseMoveEvent(int x, int y)
        {
            ET_INFO("[ Mouse move event ] Pos:", x, y);
        }

        ~MouseMoveEvent() override = default;

        EventType GetType() const override { return EventType::MouseMoveEvent; }
};

class MouseScrollEvent : public Event
{
    public:
        MouseScrollEvent(int xoffset, int yoffset)
        {
            ET_INFO("[ Mouse scroll event ] Offset:", xoffset, yoffset);
        }

        ~MouseScrollEvent() override = default;

        EventType GetType() const override { return EventType::MouseScrollEvent; }
};