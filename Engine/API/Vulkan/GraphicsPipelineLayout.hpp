#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace Eternity
{
    struct VertexInput
    {
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};

        VertexInput(const std::vector<VkVertexInputBindingDescription>& vertexDescriptions, const std::vector<VkVertexInputAttributeDescription>& attribDescriptions)
        {
            vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertexInputInfo.vertexBindingDescriptionCount   = static_cast<uint32_t>(vertexDescriptions.size());
            vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribDescriptions.size());
            vertexInputInfo.pVertexBindingDescriptions      = vertexDescriptions.data();
            vertexInputInfo.pVertexAttributeDescriptions    = attribDescriptions.data();
        }
    };
    
    class GraphicsPipelineLayout
    {
        private:

        public:
            GraphicsPipelineLayout() = default;
            ~GraphicsPipelineLayout() = default;
    };
} // namespace Eternity
