#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace Eternity
{

    
    class Device;

    class GraphicsPipelineLayout
    {
        private:
            const Device&       m_Device;
            VkPipelineLayout    m_PipelineLayout;
        public:
            GraphicsPipelineLayout(const Device& device, const VkDescriptorSetLayout& layout);
            ~GraphicsPipelineLayout();

            operator VkPipelineLayout() const { return m_PipelineLayout; }
    };
} // namespace Eternity
