#pragma once
#include <string>

namespace Eternity
{
    enum class EventType
    {
        WindowCloseEvent,
        WindowResizeEvent,
        KeyEvent,
        MouseButtonEvent,
        MouseMoveEvent
    };

    class Event
    {
        private:
        public:
            virtual ~Event() = default;
            virtual EventType GetType() const = 0;
    };

    enum class PressState
    {
        None, 
        Press,
        Held,
        Released
    };
}

#include "KeyEvent.hpp"
#include "MouseEvent.hpp"
#include "WindowEvent.hpp"