#pragma once

#include "Event.hpp"
#include "MouseCodes.hpp"
#include "Base.hpp"

namespace Eternity
{
    class MouseButtonEvent : public Event
    {
        private:
            struct MouseButtonInfo
            {
                MouseCode   button;
                PressState  state;
            };

            MouseButtonInfo m_Button;            
        public:
            MouseButtonEvent(int button, int action)
            {
                m_Button.button = button;
                if (action == GLFW_PRESS)
                    m_Button.state = PressState::Press;
                else
                if (action == GLFW_RELEASE)
                    m_Button.state = PressState::Released;
            }

            ~MouseButtonEvent() override
            {
                ET_CORE_INFO("[ Mouse button event ] Button: ", m_Button.button, "State: ", (int)m_Button.state);
            }

            MouseButtonInfo GetButton() const { return m_Button; }
            EventType GetType() const override { return EventType::MouseButtonEvent; }
    };

    class MouseMoveEvent : public Event
    {
        private:
            struct MousePositionInfo
            {
                double x;
                double y;
            };

            MousePositionInfo m_MousePosition;
        public:
            MouseMoveEvent(double xpos, double ypos)
            {
                m_MousePosition.x = xpos;
                m_MousePosition.y = ypos;
            }
            
            ~MouseMoveEvent() override
            {
                ET_CORE_INFO("[ Mouse move event ] Pos: x: ", m_MousePosition.x, "y: ", m_MousePosition.y);
            }

            MousePositionInfo   GetPosition() const { return m_MousePosition; }
            EventType           GetType() const override { return EventType::MouseMoveEvent; }
    };
}