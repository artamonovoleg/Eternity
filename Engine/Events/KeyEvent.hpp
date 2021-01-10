#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "Event.hpp"
#include "KeyCodes.hpp"
#include "Base.hpp"

namespace Eternity
{
    class KeyEvent : public Event
    {
        private:
            struct KeyInfo
            {
                KeyCode         key;
                PressState      state;
            };

            KeyInfo m_Key;
        public:
            KeyEvent(int key, int action)
            {
                m_Key.key = key;
                if (action == GLFW_PRESS)
                    m_Key.state = PressState::Press;
                else
                if (action == GLFW_RELEASE)
                    m_Key.state = PressState::Released;
                else
                if (action == GLFW_REPEAT)
                    m_Key.state = PressState::Held;
            }

            ~KeyEvent() override 
            {
                ET_CORE_INFO("[ Key event ] Key: ", m_Key.key, "State: ", (int)m_Key.state);
            }

            KeyInfo     GetKey() const { return m_Key; }
            EventType   GetType() const override { return EventType::KeyEvent; }
    };
}