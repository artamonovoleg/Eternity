#pragma once

#include <memory>
#include <vulkan/vulkan.h>

namespace Eternity
{
    class Device;
    class CommandPool;

    class Buffer
    {
        protected:
            const Device&   m_Device;
            VkBuffer        m_Buffer;
            VkDeviceMemory  m_Memory;
            VkDeviceSize    m_Size;
        public:
            Buffer(const Device& device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
            ~Buffer();

            void MapMemory(void** data);
            void UnmapMemory();

            operator VkBuffer() { return m_Buffer; }
            operator VkBuffer() const { return m_Buffer; }
    };
    
    /// Vuffer create helpers
    std::shared_ptr<Buffer> CreateVertexBuffer(const CommandPool& commandPool, const void* data, VkDeviceSize size);
    std::shared_ptr<Buffer> CreateIndexBuffer(const CommandPool& commandPool, const void* data, VkDeviceSize size);
    
} // namespace Eternity
