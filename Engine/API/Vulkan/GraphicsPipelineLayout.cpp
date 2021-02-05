#include "GraphicsPipelineLayout.hpp"
#include "Device.hpp"
#include "VkCheck.hpp"
#include "Base.hpp"

namespace Eternity
{
    GraphicsPipelineLayout::GraphicsPipelineLayout(const Device& device, const VkDescriptorSetLayout& layout)
        : m_Device(device)
    {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType            = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount   = 1;
        pipelineLayoutInfo.pSetLayouts      = &layout;

        VkCheck(vkCreatePipelineLayout(m_Device, &pipelineLayoutInfo, nullptr, &m_PipelineLayout));
        ET_TRACE("Pipeline layout created");
    };

    GraphicsPipelineLayout::~GraphicsPipelineLayout()
    {
        vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);
        ET_TRACE("Pipeline layout destroyed");
    }
} // namespace Eternity
