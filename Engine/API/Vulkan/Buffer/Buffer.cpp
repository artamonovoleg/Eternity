#include <cstring>
#include "Buffer.hpp"
#include "Device.hpp"
#include "CommandPool.hpp"
#include "VkCheck.hpp"
#include "Utils.hpp"
#include "Base.hpp"

namespace Eternity
{
    Buffer::Buffer(const Device& device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
        : m_Device(device), m_Size(size)
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VkCheck(vkCreateBuffer(m_Device, &bufferInfo, nullptr, &m_Buffer));

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_Device, m_Buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(m_Device.GetPhysicalDevice(), memRequirements.memoryTypeBits, properties);

        VkCheck(vkAllocateMemory(m_Device, &allocInfo, nullptr, &m_Memory));

        vkBindBufferMemory(m_Device, m_Buffer, m_Memory, 0);
    }

    Buffer::~Buffer()
    {
        vkFreeMemory(m_Device, m_Memory, nullptr);
        vkDestroyBuffer(m_Device, m_Buffer, nullptr);
    }

    void Buffer::MapMemory(void** data)
    {
        vkMapMemory(m_Device, m_Memory, 0, m_Size, 0, data);
    }

    void Buffer::UnmapMemory()
    {
        vkUnmapMemory(m_Device, m_Memory);
    }

    /// Vuffer create helpers
    std::shared_ptr<Buffer> CreateVertexBuffer(const CommandPool& commandPool, const void* verticesData, VkDeviceSize size)
    {
        Buffer stagingBuffer (commandPool.GetDevice(), size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        
        void* data;
        stagingBuffer.MapMemory(&data);
            std::memcpy(data, verticesData, (size_t) size);
        stagingBuffer.UnmapMemory();

        auto vertexBuffer = std::make_shared<Eternity::Buffer>(commandPool.GetDevice(), size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        commandPool.CopyBuffer(stagingBuffer, *vertexBuffer, size);

        return vertexBuffer;
    }

    std::shared_ptr<Buffer> CreateIndexBuffer(const CommandPool& commandPool, const void* indicesData, VkDeviceSize size)
    {
        Buffer stagingBuffer (commandPool.GetDevice(), size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        void* data;
        stagingBuffer.MapMemory(&data);
            std::memcpy(data, indicesData, (size_t) size);
        stagingBuffer.UnmapMemory();

        auto indexBuffer = std::make_shared<Eternity::Buffer>(commandPool.GetDevice(), size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        
        commandPool.CopyBuffer(stagingBuffer, *indexBuffer, size);
        
        return indexBuffer;
    }

} // namespace Eternity
