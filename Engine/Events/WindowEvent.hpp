#pragma once
#include "Event.hpp"
#include "Base.hpp"

namespace Eternity
{
    class WindowCloseEvent : public Event
    {
        private:
        public:
            WindowCloseEvent() = default;
            ~WindowCloseEvent() override
            {
                ET_CORE_INFO("[ Window close event ]");
            }

            EventType GetType() const override { return EventType::WindowCloseEvent; }
    };

    class WindowResizeEvent : public Event
    {
        private:
            struct WindowSize
            {
                int width;
                int height;
            };
            
            const WindowSize m_Size;
        public:
            WindowResizeEvent(int width, int height)
                : m_Size({ width, height }){}
            ~WindowResizeEvent() override 
            {
                ET_CORE_INFO("[ Window resize event ] Width: ", m_Size.width, " Height: ", m_Size.height);
            }

            EventType GetType() const override { return EventType::WindowResizeEvent; }
    };
}