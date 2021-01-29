#include "EventSystem.hpp"
#include "Window.hpp"

namespace Eternity
{
    std::unordered_map<EventType, std::list<std::function<void(const Event&)>>> EventSystem::listeners;

    void EventSystem::Init()
    {
        glfwSetKeyCallback(Eternity::GetWindow(), [](GLFWwindow*, int key, int scancode, int action, int mods)
        { 
            EventSystem::SendEvent(KeyEvent(key, action));
        });

        glfwSetMouseButtonCallback(Eternity::GetWindow(), [](GLFWwindow*, int button, int action, int mods)
        { 
            EventSystem::SendEvent(MouseEvent(button, action));
        });

        glfwSetScrollCallback(Eternity::GetWindow(), [](GLFWwindow* window, double xoffset, double yoffset)
        { 
            EventSystem::SendEvent(MouseScrollEvent(xoffset, yoffset));
        });

        glfwSetCursorPosCallback(Eternity::GetWindow(), [](GLFWwindow* window, double xpos, double ypos)
        { 
            EventSystem::SendEvent(MouseMoveEvent(xpos, ypos));
        });

        glfwSetFramebufferSizeCallback(Eternity::GetWindow(), [](GLFWwindow*, int width, int height)
        { 
            EventSystem::SendEvent(WindowResizeEvent(width, height));
        });
        // glfwSetWindowCloseCallback(glfwGetCurrentContext(), [](GLFWwindow* window)
        // { OnWindowCloseAction(); });
    }

    void EventSystem::SendEvent(const Event& ev)
    {
        for (auto const& listener : listeners[ev.GetType()])
            listener(ev);
    }

    void EventSystem::AddListener(EventType type, std::function<void(const Event&)> const& listener)
    {
        listeners[type].push_back(listener);
    }

    void EventSystem::PollEvents()
    {
        glfwPollEvents();
    }
} // namespace Eternity