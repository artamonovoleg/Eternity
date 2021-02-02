#include "UniformBuffer.hpp"
#include "Device.hpp"

namespace Eternity
{
    UniformBuffer::UniformBuffer(const Device& device, VkDeviceSize size)
        : Buffer(device, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {}

    void UniformBuffer::MapMemory(VkDeviceSize size, void** data)
    {
        vkMapMemory(m_Device, m_Memory, 0, size, 0, data);
    }

} // namespace Eternity
