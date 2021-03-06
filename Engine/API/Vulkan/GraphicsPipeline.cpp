#include "GraphicsPipeline.hpp"
#include "Device.hpp"
#include "RenderPass.hpp"
#include "Shader.hpp"
#include "GraphicsPipelineLayout.hpp"
#include "VkCheck.hpp"
#include "Base.hpp"

namespace Eternity
{
    GraphicsPipeline::GraphicsPipeline(const Device& device, const RenderPass& renderPass, const ShaderStage& shaderStage, const VertexInput& vertexInput, const GraphicsPipelineLayout& layout, VkExtent2D extent, VkPrimitiveTopology topology /*= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST */)
        : m_Device(device)
    {
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType                     = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology                  = topology;
        inputAssembly.primitiveRestartEnable    = VK_FALSE;

        VkViewport viewport{};
        viewport.x          = 0.0f;
        viewport.y          = 0.0f;
        viewport.width      = (float) extent.width;
        viewport.height     = (float) extent.height;
        viewport.minDepth   = 0.0f;
        viewport.maxDepth   = 1.0f;

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = extent;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType             = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount     = 1;
        viewportState.pViewports        = &viewport;
        viewportState.scissorCount      = 1;
        viewportState.pScissors         = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType                    = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable         = VK_FALSE;
        rasterizer.rasterizerDiscardEnable  = VK_FALSE;
        rasterizer.polygonMode              = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth                = 1.0f;
        rasterizer.cullMode                 = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace                = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable          = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable   = VK_FALSE;
        multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType                  = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable        = VK_TRUE;
        depthStencil.depthWriteEnable       = VK_TRUE;
        depthStencil.depthCompareOp         = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable  = VK_FALSE;
        depthStencil.stencilTestEnable      = VK_FALSE;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable    = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable     = VK_FALSE;
        colorBlending.logicOp           = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount   = 1;
        colorBlending.pAttachments      = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType                  = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount             = shaderStage.GetStageCount();
        pipelineInfo.pStages                = shaderStage.GetStages();
        pipelineInfo.pVertexInputState      = &vertexInput.vertexInputInfo;
        pipelineInfo.pInputAssemblyState    = &inputAssembly;
        pipelineInfo.pViewportState         = &viewportState;
        pipelineInfo.pRasterizationState    = &rasterizer;
        pipelineInfo.pMultisampleState      = &multisampling;
        pipelineInfo.pDepthStencilState     = &depthStencil;
        pipelineInfo.pColorBlendState       = &colorBlending;
        pipelineInfo.layout                 = layout;
        pipelineInfo.renderPass             = renderPass;
        pipelineInfo.subpass                = 0;
        pipelineInfo.basePipelineHandle     = VK_NULL_HANDLE;

        VkCheck(vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_Pipeline));
        ET_TRACE("Pipeline created");
    }

    GraphicsPipeline::~GraphicsPipeline()
    {
        vkDestroyPipeline(m_Device, m_Pipeline, nullptr);
        ET_TRACE("Pipeline destroyed");
    }
} // namespace Eternity
