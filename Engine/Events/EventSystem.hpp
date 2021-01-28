#pragma once

#include <unordered_map>
#include <functional>
#include <list>
#include <GLFW/glfw3.h>
#include "Event.hpp"

namespace Eternity
{
    class EventSystem
    {
        private:
            static std::unordered_map<EventType, std::list<std::function<void(const Event&)>>> listeners;

            static void SendEvent(const Event& ev);
        public:
            static void Init();
            static void PollEvents();
            static void AddListener(EventType type, std::function<void(const Event&)> const& listener);
    };
}