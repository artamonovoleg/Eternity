#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace Eternity
{
    class Device;
    class Swapchain;
    enum class DescriptorType;
    
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
