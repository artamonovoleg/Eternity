#pragma once

#include <memory>
#include <vulkan/vulkan.h>

namespace Eternity
{
    class Device;
    
    class Buffer
    {
        private:
            const Device&   m_Device;
            VkBuffer        m_Buffer;
            VkDeviceMemory  m_Memory;
            VkDeviceSize    m_Size;
        public:
            Buffer(const Device& device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
            ~Buffer();

            void MapMemory(void** data);
            void MapMemory(VkDeviceSize size, void** data);
            void UnmapMemory();

            operator VkBuffer() { return m_Buffer; }
            operator VkBuffer() const { return m_Buffer; }
    };
} // namespace Eternity
