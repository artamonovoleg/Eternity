#include "EventSystem.hpp"

#include "Window.hpp"

namespace Eternity
{
    std::unordered_map<EventType, std::list<std::function<void(const Event&)>>> listeners;

    void EventSystem::HandleEvent(const Event& event)
    {
        for (const auto& listener : listeners[event.GetType()])
            listener(event);
    }

    void EventSystem::Init()
    {
        glfwSetKeyCallback(Eternity::GetCurrentWindow(), [](GLFWwindow*, int key, int scancode, int action, int)
        {
            HandleEvent(KeyEvent(key, action));
        });

        glfwSetMouseButtonCallback(Eternity::GetCurrentWindow(), [](GLFWwindow*, int button, int action, int mods)
        {
            HandleEvent(MouseButtonEvent(button, action));
        });

        glfwSetWindowCloseCallback(Eternity::GetCurrentWindow(), [](GLFWwindow*) 
        {
            HandleEvent(WindowCloseEvent());
        });

        glfwSetWindowSizeCallback(Eternity::GetCurrentWindow(), [](GLFWwindow*, int width, int height)
        {
            HandleEvent(WindowResizeEvent(width, height));
        });

        glfwSetCursorPosCallback(Eternity::GetCurrentWindow(), [](GLFWwindow*, double xpos, double ypos)
        {
            HandleEvent(MouseMoveEvent(xpos, ypos));
        });
    }

    void EventSystem::PollEvents()
    {
        glfwPollEvents();
    }

    void EventSystem::AddListener(EventType type, std::function<void(const Event&)> listener)
    {
        listeners[type].push_back(listener);
    }
}