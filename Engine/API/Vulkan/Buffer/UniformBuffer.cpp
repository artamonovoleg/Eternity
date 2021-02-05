#include "UniformBuffer.hpp"
#include "Device.hpp"
#include "Descriptors.hpp"

namespace Eternity
{
    UniformBuffer::UniformBuffer(const Device& device, VkDeviceSize size)
        : Buffer(device, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {}

    void UniformBuffer::MapMemory(VkDeviceSize size, void** data)
    {
        vkMapMemory(m_Device, m_Memory, 0, size, 0, data);
    }

    WriteDescriptorSet UniformBuffer::GetWriteDescriptorSet(uint32_t binding, uint32_t count, VkDeviceSize range, VkDeviceSize offset /* = 0 */)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer   = m_Buffer;
        bufferInfo.offset   = offset;
        bufferInfo.range    = range;

        VkWriteDescriptorSet writeDescriptor{};
        writeDescriptor.sType               = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptor.dstBinding          = 0;
        writeDescriptor.dstArrayElement     = 0;
        writeDescriptor.descriptorType      = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writeDescriptor.descriptorCount     = 1;

        return WriteDescriptorSet(bufferInfo, writeDescriptor);
    }

    VkDescriptorSetLayoutBinding UniformBuffer::GetDescriptorSetLayout(uint32_t binding, uint32_t count)
    {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding            = binding;
        uboLayoutBinding.descriptorCount    = count;
        uboLayoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.pImmutableSamplers = nullptr;
        uboLayoutBinding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;

        return uboLayoutBinding;
    }
} // namespace Eternity
