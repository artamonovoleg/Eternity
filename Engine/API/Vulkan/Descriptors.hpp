#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace Eternity
{
    class Swapchain;
    class Device;

    enum class DescriptorType
    {
        Uniform, ImageSampler
    };

    class WriteDescriptorSet
    {
        private:
            VkDescriptorImageInfo   m_ImageInfo;
            VkDescriptorBufferInfo  m_BufferInfo;
            VkWriteDescriptorSet    m_Descriptor;
        public:
            WriteDescriptorSet(VkDescriptorBufferInfo bufferInfo, VkWriteDescriptorSet descriptor)
                : m_BufferInfo(bufferInfo), m_Descriptor(descriptor)
            {
                m_Descriptor.pBufferInfo = &m_BufferInfo;
            }

            WriteDescriptorSet(VkDescriptorImageInfo imageInfo, VkWriteDescriptorSet descriptor)
                : m_ImageInfo(imageInfo), m_Descriptor(descriptor)
            {
                m_Descriptor.pImageInfo = &m_ImageInfo;
            }

            ~WriteDescriptorSet() = default;

            operator VkWriteDescriptorSet() const { return m_Descriptor; }
            operator VkWriteDescriptorSet() { return m_Descriptor; }
    };

    class DescriptorSetLayout
    {
        private:
            const Device&           m_Device;

            VkDescriptorSetLayout   m_Layout;
        public:
            DescriptorSetLayout(const Device& device, const std::vector<VkDescriptorSetLayoutBinding>& bindings);
            ~DescriptorSetLayout();

            operator VkDescriptorSetLayout() const { return m_Layout; }
    };

    class DescriptorPool
    {
        private:
            const Device&       m_Device;

            VkDescriptorPool    m_DescriptorPool;
        public:
            DescriptorPool(const Swapchain& swapchain, const std::vector<DescriptorType>& types);
            ~DescriptorPool();

            operator VkDescriptorPool() { return m_DescriptorPool; }
            operator VkDescriptorPool() const { return m_DescriptorPool; }
    };
} // namespace Eternity
