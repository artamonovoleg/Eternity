#pragma once
#include <vector>
#include <vulkan/vulkan.h>

namespace Eternity
{
    class Device;

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
} // namespace Eternity
