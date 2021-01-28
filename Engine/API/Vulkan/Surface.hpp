#pragma once
#include <vulkan/vulkan.h>

namespace Eternity
{
    class Instance;

    class Surface
    {
        private:
            const Instance& m_Instance;
            VkSurfaceKHR    m_Surface;
        public:
            Surface(const Instance& instance);
            ~Surface();

            operator VkSurfaceKHR() { return m_Surface; }
            operator VkSurfaceKHR() const { return m_Surface; }
    };
} // namespace Eternity