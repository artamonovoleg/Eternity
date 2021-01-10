#pragma once

#include <unordered_map>
#include <list>
#include <functional>

#include "Event.hpp"

namespace Eternity
{
    class EventSystem
    {
        private:
            static void HandleEvent(const Event& event);
        public:
            static void Init();
            static void PollEvents();

            static void AddListener(EventType type, std::function<void(const Event&)> listener);
    };
}