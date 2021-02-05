#pragma once
#include "Buffer.hpp"

namespace Eternity
{
    class WriteDescriptorSet;

    class UniformBuffer : public Buffer
    {
        private:
        public:
            UniformBuffer(const Device& device, VkDeviceSize size);

            void MapMemory(VkDeviceSize size, void** data);

            WriteDescriptorSet GetWriteDescriptorSet(uint32_t binding, uint32_t count, VkDeviceSize range, VkDeviceSize offset = 0);

            static VkDescriptorSetLayoutBinding GetDescriptorSetLayout(uint32_t binding, uint32_t count);
    };
} // namespace Eternity
