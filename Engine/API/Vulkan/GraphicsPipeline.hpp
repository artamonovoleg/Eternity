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

    class Device;
    class RenderPass;
    class ShaderStage;
    class GraphicsPipelineLayout;

    class GraphicsPipeline
    {
        private:
            const Device&       m_Device;
            VkPipeline          m_Pipeline;
        public:
            GraphicsPipeline(
                                const Device& device, 
                                const RenderPass& renderPass,
                                const ShaderStage& shaderStage, 
                                const VertexInput& vertexInput, 
                                const GraphicsPipelineLayout& layout, 
                                VkExtent2D extent, 
                                VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
            ~GraphicsPipeline();

            operator VkPipeline() const { return m_Pipeline; }
    };
} // namespace Eternity
