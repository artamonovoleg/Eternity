#pragma once
#include <vulkan/vulkan.h>

namespace Eternity
{
    class Buffer
    {
        private:
            VkBuffer        m_Buffer;
            VkDeviceMemory  m_DeviceMemory;
        public:
            Buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, const void *data = nullptr);
            ~Buffer();
    };
} // namespace Eternity
