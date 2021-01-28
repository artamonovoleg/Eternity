#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "Surface.hpp"
#include "Window.hpp"
#include "Instance.hpp"
#include "VkCheck.hpp"
#include "Base.hpp"

namespace Eternity
{
    Surface::Surface(const Instance& instance)
        : m_Instance(instance)
    {
        VkCheck(glfwCreateWindowSurface(instance, Eternity::GetWindow(), nullptr, &m_Surface));
        ET_TRACE("Surface created");
    }

    Surface::~Surface()
    {
        vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
        ET_TRACE("Surface destroyed");
    }
} // namespace Eternity
