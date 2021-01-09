#include "VulkanHelper.hpp"

namespace vkh
{
    VkPipelineShaderStageCreateInfo                 PipelineShaderStageCreateInfo(const VkShaderStageFlagBits& stage, const VkShaderModule& shaderModule) 
    {
		VkPipelineShaderStageCreateInfo pssCI
        {
		    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,

            //shader stage
            .stage = stage,
            //module containing the code for this shader stage
            .module = shaderModule,
            //the entry point of the shader
            .pName = "main"
        };

		return pssCI;
	}

    VkPipelineVertexInputStateCreateInfo            VertexInputStateCreateInfo() 
    {
		VkPipelineVertexInputStateCreateInfo pvisCI
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,

            //no vertex bindings or attributes
            .vertexBindingDescriptionCount = 0,
            .vertexAttributeDescriptionCount = 0
        };

		return pvisCI;
    }

    VkPipelineInputAssemblyStateCreateInfo          InputAssemblyCreateInfo(const VkPrimitiveTopology& topology) 
    {
		VkPipelineInputAssemblyStateCreateInfo piasCI
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .pNext = nullptr,

            .topology = topology,
            //we are not going to use primitive restart on the entire tutorial so leave it on false
            .primitiveRestartEnable = VK_FALSE
        };

		return piasCI;
	}

    VkPipelineRasterizationStateCreateInfo          RasterizationStateCreateInfo(const VkPolygonMode& polygonMode)
	{
		VkPipelineRasterizationStateCreateInfo prsCI 
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,

            .depthClampEnable = VK_FALSE,
            //discards all primitives before the rasterization stage if enabled which we don't want
            .rasterizerDiscardEnable = VK_FALSE,

            .polygonMode = polygonMode,
            //no backface cull
            .cullMode = VK_CULL_MODE_NONE,
            .frontFace = VK_FRONT_FACE_CLOCKWISE,
            //no depth bias
            .depthBiasEnable = VK_FALSE,
            .depthBiasConstantFactor = 0.0f,
            .depthBiasClamp = 0.0f,
            .depthBiasSlopeFactor = 0.0f,
            .lineWidth = 1.0f,
        };


		return prsCI;
	}

    VkPipelineMultisampleStateCreateInfo            MultisamplingStateCreateInfo()
	{
		VkPipelineMultisampleStateCreateInfo pmsCI 
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,

            //multisampling defaulted to no multisampling (1 sample per pixel)
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
            .minSampleShading = 1.0f,
            .pSampleMask = nullptr,
            .alphaToCoverageEnable = VK_FALSE,
            .alphaToOneEnable = VK_FALSE
        };

		return pmsCI;
	}

    VkPipelineColorBlendAttachmentState             ColorBlendAttachmentState()
    {
		VkPipelineColorBlendAttachmentState colorBlendAttachment
        {
		    .blendEnable = VK_FALSE,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
        };

		return colorBlendAttachment;
	}

    VkPipeline                              PipelineBuilder::BuildPipeline(const VkDevice& device, const VkRenderPass& pass)
    {
        VkPipeline  pipeline = VK_NULL_HANDLE;
        //make viewport state from our stored viewport and scissor.
        //at the moment we won't support multiple viewports or scissors
        VkPipelineViewportStateCreateInfo viewportState = {};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;

        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        //setup dummy color blending. We aren't using transparent objects yet
        //the blending is just "no blend", but we do write to the color attachment
        VkPipelineColorBlendStateCreateInfo colorBlending = {};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.pNext = nullptr;

        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;

        //build the actual pipeline
        //we now use all of the info structs we have been writing into into this one to create the pipeline
        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.pNext = nullptr;

        pipelineInfo.stageCount = shaderStages.size();
        pipelineInfo.pStages = shaderStages.data();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = pass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        auto res = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);
        vkh::Check(res, "Pipeline create failed");
        return pipeline;
    }

    VkPipelineLayoutCreateInfo                      PipelineLayoutCreateInfo()
    {
		VkPipelineLayoutCreateInfo pipelineLayoutCI
        {
            .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,

            //empty defaults
            .flags                  = 0,
            .setLayoutCount         = 0,
            .pSetLayouts            = nullptr,
            .pushConstantRangeCount = 0,
            .pPushConstantRanges    = nullptr,
        };

		return pipelineLayoutCI;
	}
}