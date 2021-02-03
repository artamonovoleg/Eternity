#pragma once
#include "Buffer.hpp"

namespace Eternity
{
    class UniformBuffer : public Buffer
    {
        private:
        public:
            UniformBuffer(const Device& device, VkDeviceSize size);

            void MapMemory(VkDeviceSize size, void** data);

            static VkDescriptorSetLayoutBinding GetDescriptorSetLayout(uint32_t binding, uint32_t count);
    };
} // namespace Eternity
