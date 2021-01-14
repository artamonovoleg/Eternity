#include <filesystem>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "VulkanRenderer.hpp"
#include "VulkanHelper.hpp"
#include "VulkanDebugCallback.hpp"
#include "EventSystem.hpp"

const int MAX_FRAMES_IN_FLIGHT = 2;

namespace Eternity
{
    void VulkanRenderer::InitVulkan()
    {
        Eternity::EventSystem::AddListener(EventType::WindowResizeEvent, [&](const Event& event)
        {
            m_FramebufferResized = true;            
        });

        InitInstance();
        CreateSurface();
        CreateLogicalDevice();
        CreateSwapchain();

        CreateRenderPass();
        CreateDescriptorSetLayout();
        CreateGraphicsPipeline();


        CreateCommandPool();
        CreateDepthResources();
        CreateFramebuffers();

        CreateTextureImage();
        CreateTextureImageView();
        CreateTextureSampler();
        
        LoadModel();
        CreateVertexBuffer();
        CreateIndexBuffer();
        CreateUniformBuffers();
        CreateDescriptorPool();
        CreateDescriptorSets();
        CreateCommandBuffers();

        CreateSyncObjects();
    }

    void VulkanRenderer::SetTargetModel(const std::string& modelPath, const std::string& texturePath)
    {
        m_ModelPath     = modelPath;
        m_TexturePath   = texturePath;
    }

    void VulkanRenderer::DeinitVulkan()
    {
        vkDeviceWaitIdle(m_Device);
        m_DeletionQueue.Flush();
    }

    
    void VulkanRenderer::DrawFrame()
    {
        vkWaitForFences(m_Device, 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        auto res = vkAcquireNextImageKHR(m_Device, m_Swapchain, UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);

        if (res == VK_ERROR_OUT_OF_DATE_KHR) 
        {
            RecreateSwapchain();
            return;
        } 

        if (m_ImagesInFlight[imageIndex] != VK_NULL_HANDLE) 
            vkWaitForFences(m_Device, 1, &m_ImagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);

        m_ImagesInFlight[imageIndex] = m_InFlightFences[m_CurrentFrame];

        UpdateUniformBuffer(imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType                    = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[]        = { m_ImageAvailableSemaphores[m_CurrentFrame] };
        VkPipelineStageFlags waitStages[]   = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount       = 1;
        submitInfo.pWaitSemaphores          = waitSemaphores;
        submitInfo.pWaitDstStageMask        = waitStages;

        submitInfo.commandBufferCount       = 1;
        submitInfo.pCommandBuffers          = &m_CommandBuffers[imageIndex];

        VkSemaphore signalSemaphores[]      = { m_RenderFinishedSemaphores[m_CurrentFrame] };
        submitInfo.signalSemaphoreCount     = 1;
        submitInfo.pSignalSemaphores        = signalSemaphores;

        vkResetFences(m_Device, 1, &m_InFlightFences[m_CurrentFrame]);

        if (vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, m_InFlightFences[m_CurrentFrame]) != VK_SUCCESS) 
            throw std::runtime_error("failed to submit draw command buffer!");

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType               = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount  = 1;
        presentInfo.pWaitSemaphores     = signalSemaphores;

        VkSwapchainKHR swapchains[]     = { m_Swapchain };
        presentInfo.swapchainCount      = 1;
        presentInfo.pSwapchains         = swapchains;

        presentInfo.pImageIndices       = &imageIndex;

        res = vkQueuePresentKHR(m_PresentQueue, &presentInfo);

        if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || m_FramebufferResized)
        {
            m_FramebufferResized = false;
            RecreateSwapchain();
        }

        m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void VulkanRenderer::RecreateSwapchain()
    {
        int width = 0, height = 0;
        glfwGetFramebufferSize(Eternity::GetCurrentWindow(), &width, &height);
        while (width == 0 || height == 0) 
        {
            glfwGetFramebufferSize(Eternity::GetCurrentWindow(), &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(m_Device);
        CleanupSwapchain();

        CreateSwapchain();
        CreateRenderPass();
        CreateGraphicsPipeline();
        CreateDepthResources();
        CreateFramebuffers();
        CreateUniformBuffers();
        CreateDescriptorPool();
        CreateDescriptorSets();
        CreateCommandBuffers();
    }

    void VulkanRenderer::CleanupSwapchain()
    {
        vkDestroyImageView(m_Device, m_DepthImageView, nullptr);
        vkDestroyImage(m_Device, m_DepthImage, nullptr);
        vkFreeMemory(m_Device, m_DepthImageMemory, nullptr);

        for (auto framebuffer : m_SwapchainFramebuffers)
            vkDestroyFramebuffer(m_Device, framebuffer, nullptr);

        vkFreeCommandBuffers(m_Device, m_CommandPool, static_cast<uint32_t>(m_CommandBuffers.size()), m_CommandBuffers.data());

        vkDestroyPipeline(m_Device, m_GraphicsPipeline, nullptr);
        vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);
        vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);

        for (auto imageView : m_ImageViews)
            vkDestroyImageView(m_Device, imageView, nullptr);

        vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);

        for (size_t i = 0; i < m_Images.size(); i++) 
        {
            vkDestroyBuffer(m_Device, m_UniformBuffers[i], nullptr);
            vkFreeMemory(m_Device, m_UniformBuffersMemory[i], nullptr);
        }

        vkDestroyDescriptorPool(m_Device, m_DescriptorPool, nullptr);
    }

    void VulkanRenderer::InitInstance()
    {
        VkApplicationInfo   appInfo
        {
            .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName   = "ObjLoader",
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName        = "No engine", 
            .engineVersion      = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion         = VK_API_VERSION_1_2 
        };
        
        auto inst_ret = vkh::BuildInstance(appInfo, true, DebugCallback);
        m_Instance          = inst_ret.first;
        m_DebugMessenger    = inst_ret.second;

        m_DeletionQueue.PushDeleter([=]()
        {
            vkh::DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
            vkDestroyInstance(m_Instance, nullptr);
        });
    }
    
    void VulkanRenderer::CreateSurface()
    {
        auto res = glfwCreateWindowSurface(m_Instance, Eternity::GetCurrentWindow(), nullptr, &m_Surface);
        vkh::Check(res, "Surface create failed");
        m_DeletionQueue.PushDeleter([=]()
        {
            vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
        });
    }

    void VulkanRenderer::CreateLogicalDevice()
    {
        m_GPU       = vkh::SelectPhysicalDevice(m_Instance, m_Surface);
        m_Device    = vkh::BuildDevice(m_GPU, m_Surface);

        auto indices        = vkh::FindQueueFamilies(m_GPU, m_Surface);
        m_GraphicsFamily    = indices.graphicsFamily.value();
        m_PresentFamily     = indices.presentFamily.value();

        vkGetDeviceQueue(m_Device, m_GraphicsFamily, 0, &m_GraphicsQueue);
        vkGetDeviceQueue(m_Device, m_PresentFamily, 0, &m_PresentQueue);

        m_DeletionQueue.PushDeleter([=]()
        {
            vkDestroyDevice(m_Device, nullptr);
        });
    }

    void VulkanRenderer::CreateSwapchain()
    {
        auto swapchain_ret  = vkh::BuildSwapchain(Eternity::GetCurrentWindow(), m_GPU, m_Surface, m_Device);

        m_Swapchain         = swapchain_ret.swapchain;
        m_Images            = swapchain_ret.images;
        m_ImageFormat       = swapchain_ret.imageFormat;
        m_Extent            = swapchain_ret.extent;
        m_ImageViews        = swapchain_ret.imageViews;
        
        m_DeletionQueue.PushDeleter([&]()
        {
            for (const auto& imageView : m_ImageViews)
                vkDestroyImageView(m_Device, imageView, nullptr);
            vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);
        });
    }

    void VulkanRenderer::CreateRenderPass()
    {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format          = m_ImageFormat;
        colorAttachment.samples         = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp          = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp         = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp   = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp  = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout   = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout     = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment   = 0;
        colorAttachmentRef.layout       = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription depthAttachment{};
        depthAttachment.format          = vkh::FindDepthFormat(m_GPU);
        depthAttachment.samples         = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp          = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp         = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp   = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp  = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout   = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment   = 1;
        depthAttachmentRef.layout       = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount    = 1;
        subpass.pColorAttachments       = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass           = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass           = 0;
        dependency.srcStageMask         = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask        = 0;
        dependency.dstStageMask         = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask        = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

        VkRenderPassCreateInfo renderPassCI{};
        renderPassCI.sType              = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCI.attachmentCount    = static_cast<uint32_t>(attachments.size());
        renderPassCI.pAttachments       = attachments.data();
        renderPassCI.subpassCount       = 1;
        renderPassCI.pSubpasses         = &subpass;
        renderPassCI.dependencyCount    = 1;
        renderPassCI.pDependencies      = &dependency;

        auto res = vkCreateRenderPass(m_Device, &renderPassCI, nullptr, &m_RenderPass);
        vkh::Check(res, "Render pass create failed");

        m_DeletionQueue.PushDeleter([=]()
        {
            vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);
        });
    }

    void VulkanRenderer::CreateDescriptorSetLayout()
    {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding            = 0;
        uboLayoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount    = 1;
        uboLayoutBinding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding            = 1;
        samplerLayoutBinding.descriptorCount    = 1;
        samplerLayoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };

        VkDescriptorSetLayoutCreateInfo layoutCI{};
        layoutCI.sType          = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutCI.bindingCount   = static_cast<uint32_t>(bindings.size());
        layoutCI.pBindings      = bindings.data();

        vkh::Check(vkCreateDescriptorSetLayout(m_Device, &layoutCI, nullptr, &m_DescriptorSetLayout), "Create descriptor set layout failed");
        m_DeletionQueue.PushDeleter([&]()
        {
            vkDestroyDescriptorSetLayout(m_Device, m_DescriptorSetLayout, nullptr);
        });
    }

    void VulkanRenderer::CreateDescriptorSets()
    {
        std::vector<VkDescriptorSetLayout> layouts(m_Images.size(), m_DescriptorSetLayout);
        VkDescriptorSetAllocateInfo allocCI{};
        allocCI.sType                 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocCI.descriptorPool        = m_DescriptorPool;
        allocCI.descriptorSetCount    = static_cast<uint32_t>(m_Images.size());
        allocCI.pSetLayouts           = layouts.data();

        m_DescriptorSets.resize(m_Images.size());
        auto res = vkAllocateDescriptorSets(m_Device, &allocCI, m_DescriptorSets.data());
        vkh::Check(res, "Allocate descriptors sets");

        for (size_t i = 0; i < m_Images.size(); i++) 
        {
            VkDescriptorBufferInfo bufferCI{};
            bufferCI.buffer = m_UniformBuffers[i];
            bufferCI.offset = 0;
            bufferCI.range  = sizeof(UniformBufferObject);

            VkDescriptorImageInfo imageCI{};
            imageCI.imageLayout   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageCI.imageView     = m_TextureImageView;
            imageCI.sampler       = m_TextureSampler;

            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

            descriptorWrites[0].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet          = m_DescriptorSets[i];
            descriptorWrites[0].dstBinding      = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo     = &bufferCI;

            descriptorWrites[1].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet          = m_DescriptorSets[i];
            descriptorWrites[1].dstBinding      = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo      = &imageCI;

            vkUpdateDescriptorSets(m_Device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }

    void VulkanRenderer::CreateGraphicsPipeline()
    {
        auto vertShader = vkh::CreateShaderModule(m_Device, "../Engine/shaders/triVert.spv");
        auto fragShader = vkh::CreateShaderModule(m_Device, "../Engine/shaders/triFrag.spv");

        VkPipelineShaderStageCreateInfo vertShaderStageCI{};
        vertShaderStageCI.sType     = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageCI.stage     = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageCI.module    = vertShader;
        vertShaderStageCI.pName     = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageCI{};
        fragShaderStageCI.sType     = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageCI.stage     = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageCI.module    = fragShader;
        fragShaderStageCI.pName     = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageCI, fragShaderStageCI };

        auto bindingDescription = Vertex::GetBindingDescription();
        auto attributeDescriptions = Vertex::GetAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInputCI{};
        vertexInputCI.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputCI.vertexBindingDescriptionCount   = 1;
        vertexInputCI.pVertexBindingDescriptions      = &bindingDescription;  // Optional
        vertexInputCI.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputCI.pVertexAttributeDescriptions    = attributeDescriptions.data(); // Optional

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType                     = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology                  = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable    = VK_FALSE;

        VkViewport viewport{};
        viewport.x          = 0.0f;
        viewport.y          = 0.0f;
        viewport.width      = (float) m_Extent.width;
        viewport.height     = (float) m_Extent.height;
        viewport.minDepth   = 0.0f;
        viewport.maxDepth   = 1.0f;

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = m_Extent;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports    = &viewport;
        viewportState.scissorCount  = 1;
        viewportState.pScissors     = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType                    = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable         = VK_FALSE;
        rasterizer.rasterizerDiscardEnable  = VK_FALSE;
        rasterizer.polygonMode              = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth                = 1.0f;
        rasterizer.cullMode                 = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace                = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable          = VK_FALSE;
        rasterizer.depthBiasConstantFactor  = 0.0f; // Optional
        rasterizer.depthBiasClamp           = 0.0f; // Optional
        rasterizer.depthBiasSlopeFactor     = 0.0f; // Optional

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable   = VK_FALSE;
        multisampling.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading      = 1.0f; // Optional
        multisampling.pSampleMask           = nullptr; // Optional
        multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampling.alphaToOneEnable      = VK_FALSE; // Optional

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask         = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable            = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor    = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstColorBlendFactor    = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.colorBlendOp           = VK_BLEND_OP_ADD; // Optional
        colorBlendAttachment.srcAlphaBlendFactor    = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstAlphaBlendFactor    = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.alphaBlendOp           = VK_BLEND_OP_ADD; // Optional
        
        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable     = VK_FALSE;
        colorBlending.logicOp           = VK_LOGIC_OP_COPY; // Optional
        colorBlending.attachmentCount   = 1;
        colorBlending.pAttachments      = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f; // Optional
        colorBlending.blendConstants[1] = 0.0f; // Optional
        colorBlending.blendConstants[2] = 0.0f; // Optional
        colorBlending.blendConstants[3] = 0.0f; // Optional

        VkDynamicState dynamicStates[] = 
        {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_LINE_WIDTH
        };

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType              = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount  = 2;
        dynamicState.pDynamicStates     = dynamicStates;

        VkPipelineLayoutCreateInfo pipelineLayoutCI{};
        pipelineLayoutCI.sType                    = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCI.setLayoutCount           = 1;
        pipelineLayoutCI.pSetLayouts              = &m_DescriptorSetLayout;
        pipelineLayoutCI.pushConstantRangeCount   = 0; // Optional
        pipelineLayoutCI.pPushConstantRanges      = nullptr; // Optional

        auto res = vkCreatePipelineLayout(m_Device, &pipelineLayoutCI, nullptr, &m_PipelineLayout);
        vkh::Check(res, "Pipeline layout crate failed");

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType                  = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable        = VK_TRUE;
        depthStencil.depthWriteEnable       = VK_TRUE;
        depthStencil.depthCompareOp         = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable  = VK_FALSE;
        depthStencil.stencilTestEnable      = VK_FALSE;

        VkGraphicsPipelineCreateInfo pipelineCI{};
        pipelineCI.sType                  = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineCI.stageCount             = 2;
        pipelineCI.pStages                = shaderStages;
        pipelineCI.pVertexInputState      = &vertexInputCI;
        pipelineCI.pInputAssemblyState    = &inputAssembly;
        pipelineCI.pViewportState         = &viewportState;
        pipelineCI.pRasterizationState    = &rasterizer;
        pipelineCI.pMultisampleState      = &multisampling;
        pipelineCI.pDepthStencilState     = &depthStencil; // Optional
        pipelineCI.pColorBlendState       = &colorBlending;
        pipelineCI.pDynamicState          = nullptr; // Optional
        pipelineCI.layout                 = m_PipelineLayout;
        pipelineCI.renderPass             = m_RenderPass;
        pipelineCI.subpass                = 0;
        pipelineCI.basePipelineHandle     = VK_NULL_HANDLE; // Optional
        pipelineCI.basePipelineIndex      = -1; // Optional

        res = vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &m_GraphicsPipeline);
        vkh::Check(res, "Graphics pipeline create failed");

        m_DeletionQueue.PushDeleter([=]()
        {
            vkDestroyPipeline(m_Device, m_GraphicsPipeline, nullptr);
            vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);
        });
        
        vkDestroyShaderModule(m_Device, fragShader, nullptr);
        vkDestroyShaderModule(m_Device, vertShader, nullptr);
    }

    void VulkanRenderer::CreateFramebuffers()
    {
        m_SwapchainFramebuffers.resize(m_ImageViews.size());
        for (size_t i = 0; i < m_ImageViews.size(); i++) 
        {
            std::array<VkImageView, 2> attachments = {
                m_ImageViews[i],
                m_DepthImageView
            };

            VkFramebufferCreateInfo framebufferCI{};
            framebufferCI.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferCI.renderPass      = m_RenderPass;
            framebufferCI.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferCI.pAttachments    = attachments.data();
            framebufferCI.width           = m_Extent.width;
            framebufferCI.height          = m_Extent.height;
            framebufferCI.layers          = 1;

            auto res = vkCreateFramebuffer(m_Device, &framebufferCI, nullptr, &m_SwapchainFramebuffers[i]);
            vkh::Check(res, "Swapchain framebuffers failed");

            m_DeletionQueue.PushDeleter([=]()
            {
                vkDestroyFramebuffer(m_Device, m_SwapchainFramebuffers[i], nullptr);
            });
        }
    }

    void VulkanRenderer::CreateCommandPool()
    {
        VkCommandPoolCreateInfo poolCI{};
        poolCI.sType              = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolCI.queueFamilyIndex   = m_GraphicsFamily;
        poolCI.flags              = 0; // Optional

        auto res = vkCreateCommandPool(m_Device, &poolCI, nullptr, &m_CommandPool);
        vkh::Check(res, "Command pool create failed");

        m_DeletionQueue.PushDeleter([&]()
        {
            vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
        });
    }

    void VulkanRenderer::CreateTextureImage()
    {
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(m_TexturePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        m_MipLevels     = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

        VkDeviceSize imageSize = texWidth * texHeight * 4;

        ET_CORE_ASSERT(pixels, "Failed to load texture image!");

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        vkh::CreateBuffer(m_Device, m_GPU, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(m_Device, stagingBufferMemory, 0, imageSize, 0, &data);
            std::memcpy(data, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(m_Device, stagingBufferMemory);

        stbi_image_free(pixels);
        vkh::CreateImage(m_Device, m_GPU, texWidth, texHeight, m_MipLevels, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_TextureImage, m_TextureImageMemory);

        TransitionImageLayout(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_MipLevels);
        CopyBufferToImage(stagingBuffer, m_TextureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
        // TransitionImageLayout(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_MipLevels);

        vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
        vkFreeMemory(m_Device, stagingBufferMemory, nullptr);

        GenerateMipmaps(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, m_MipLevels);

        m_DeletionQueue.PushDeleter([=]()
        {
            vkDestroyImage(m_Device, m_TextureImage, nullptr);
            vkFreeMemory(m_Device, m_TextureImageMemory, nullptr);
        });
    }

    VkImageView VulkanRenderer::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels)
    {
        VkImageViewCreateInfo viewCI{};
        viewCI.sType                            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewCI.image                            = image;
        viewCI.viewType                         = VK_IMAGE_VIEW_TYPE_2D;
        viewCI.format                           = format;
        viewCI.subresourceRange.aspectMask      = aspectFlags;
        viewCI.subresourceRange.baseMipLevel    = 0;
        viewCI.subresourceRange.levelCount      = mipLevels;
        viewCI.subresourceRange.baseArrayLayer  = 0;
        viewCI.subresourceRange.layerCount      = 1;

        VkImageView imageView;
        vkh::Check(vkCreateImageView(m_Device, &viewCI, nullptr, &imageView), "Failed to create image view");

        return imageView;
    }


    void VulkanRenderer::CreateTextureImageView()
    {
        m_TextureImageView = CreateImageView(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, m_MipLevels);

        m_DeletionQueue.PushDeleter([=]()
        {
            vkDestroyImageView(m_Device, m_TextureImageView, nullptr);
        });
    }

    void VulkanRenderer::CreateTextureSampler()
    {
        VkSamplerCreateInfo samplerCI{};
        samplerCI.sType                     = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCI.magFilter                 = VK_FILTER_LINEAR;
        samplerCI.minFilter                 = VK_FILTER_LINEAR;
        samplerCI.addressModeU              = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCI.addressModeV              = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCI.addressModeW              = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerCI.anisotropyEnable          = VK_TRUE;
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(m_GPU, &properties);
        samplerCI.maxAnisotropy             = properties.limits.maxSamplerAnisotropy;
        samplerCI.borderColor               = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerCI.unnormalizedCoordinates   = VK_FALSE;
        samplerCI.compareEnable             = VK_FALSE;
        samplerCI.compareOp                 = VK_COMPARE_OP_ALWAYS;
        samplerCI.mipmapMode                = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerCI.mipLodBias                = 0.0f;
        samplerCI.minLod                    = 0.0f;
        samplerCI.maxLod                    = 0.0f;
        
        auto res = vkCreateSampler(m_Device, &samplerCI, nullptr, &m_TextureSampler);
        vkh::Check(res, "Sampler create failed");

        m_DeletionQueue.PushDeleter([=]()
        {
            vkDestroySampler(m_Device, m_TextureSampler, nullptr);
        });
    }

    void VulkanRenderer::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
    {
        VkCommandBuffer commandBuffer = vkh::BeginSingleTimeCommands(m_Device, m_CommandPool);

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = {0, 0, 0};
        region.imageExtent = {
            width,
            height,
            1
        };

        vkCmdCopyBufferToImage(
            commandBuffer,
            buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
        );

        vkh::EndSingleTimeCommands(m_Device, m_CommandPool, m_GraphicsQueue, commandBuffer);
    }

    void VulkanRenderer::GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels)
    {
        VkCommandBuffer commandBuffer = vkh::BeginSingleTimeCommands(m_Device, m_CommandPool);

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image = image;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = 1;

        int32_t mipWidth = texWidth;
        int32_t mipHeight = texHeight;

        for (uint32_t i = 1; i < mipLevels; i++) 
        {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier);


            VkImageBlit blit{};
            blit.srcOffsets[0] = { 0, 0, 0 };
            blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
            blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = 1;
            blit.dstOffsets[0] = { 0, 0, 0 };
            blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
            blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = 1;

            vkCmdBlitImage(commandBuffer,
            image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &blit,
            VK_FILTER_LINEAR);

            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier);

            if (mipWidth > 1) mipWidth /= 2;
            if (mipHeight > 1) mipHeight /= 2;
        }

        barrier.subresourceRange.baseMipLevel = mipLevels - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        vkh::EndSingleTimeCommands(m_Device, m_CommandPool, m_GraphicsQueue, commandBuffer);
    }

    void VulkanRenderer::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels)
    {
        VkCommandBuffer commandBuffer = vkh::BeginSingleTimeCommands(m_Device, m_CommandPool);

        VkImageMemoryBarrier barrier{};
        barrier.sType       = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout   = oldLayout;
        barrier.newLayout   = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = mipLevels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.srcAccessMask = 0; // TODO
        barrier.dstAccessMask = 0; // TODO

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) 
        {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

            if (HasStencilComponent(format)) 
            {
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        } 
        else 
        {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) 
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } 
        else 
        if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } 
        else 
        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) 
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        }

        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        vkh::EndSingleTimeCommands(m_Device, m_CommandPool, m_GraphicsQueue, commandBuffer);
    }

    bool VulkanRenderer::HasStencilComponent(VkFormat format)
    {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    void VulkanRenderer::CreateDepthResources()
    {
        VkFormat depthFormat = vkh::FindDepthFormat(m_GPU);
        vkh::CreateImage(m_Device, m_GPU, m_Extent.width, m_Extent.height, 1, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_DepthImage, m_DepthImageMemory);
        m_DepthImageView = CreateImageView(m_DepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
        TransitionImageLayout(m_DepthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);

        m_DeletionQueue.PushDeleter([=]()
        {
            vkDestroyImage(m_Device, m_DepthImage, nullptr);
            vkDestroyImageView(m_Device, m_DepthImageView, nullptr);
        });
    }

    void VulkanRenderer::CreateVertexBuffer()
    {
        VkDeviceSize bufferSize = sizeof(m_Vertices[0]) * m_Vertices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        vkh::CreateBuffer(m_Device, m_GPU, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(m_Device, stagingBufferMemory, 0, bufferSize, 0, &data);
            std::memcpy(data, m_Vertices.data(), (size_t) bufferSize);
        vkUnmapMemory(m_Device, stagingBufferMemory);

        vkh::CreateBuffer(m_Device, m_GPU, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_VertexBuffer, m_VertexBufferMemory);

        vkh::CopyBuffer(m_Device, m_CommandPool, m_GraphicsQueue, stagingBuffer, m_VertexBuffer, bufferSize);
        vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
        vkFreeMemory(m_Device, stagingBufferMemory, nullptr);

        m_DeletionQueue.PushDeleter([&]()
        {
            vkDestroyBuffer(m_Device, m_VertexBuffer, nullptr);
            vkFreeMemory(m_Device, m_VertexBufferMemory, nullptr);
        });
    }

    void VulkanRenderer::CreateIndexBuffer()
    {
        VkDeviceSize bufferSize = sizeof(m_Indices[0]) * m_Indices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        vkh::CreateBuffer(m_Device, m_GPU, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(m_Device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, m_Indices.data(), (size_t) bufferSize);
        vkUnmapMemory(m_Device, stagingBufferMemory);

        vkh::CreateBuffer(m_Device, m_GPU, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_IndexBuffer, m_IndexBufferMemory);

        vkh::CopyBuffer(m_Device, m_CommandPool, m_GraphicsQueue, stagingBuffer, m_IndexBuffer, bufferSize);

        vkDestroyBuffer(m_Device, stagingBuffer, nullptr);
        vkFreeMemory(m_Device, stagingBufferMemory, nullptr);
        
        m_DeletionQueue.PushDeleter([=]()
        {
            vkDestroyBuffer(m_Device, m_IndexBuffer, nullptr);
            vkFreeMemory(m_Device, m_IndexBufferMemory, nullptr);
        });
    }

    void VulkanRenderer::CreateUniformBuffers()
    {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        m_UniformBuffers.resize(m_Images.size());
        m_UniformBuffersMemory.resize(m_Images.size());

        for (size_t i = 0; i < m_Images.size(); i++) 
        {
            vkh::CreateBuffer(m_Device, m_GPU, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_UniformBuffers[i], m_UniformBuffersMemory[i]);
            m_DeletionQueue.PushDeleter([=]()
            {
                vkDestroyBuffer(m_Device, m_UniformBuffers[i], nullptr);
                vkFreeMemory(m_Device, m_UniformBuffersMemory[i], nullptr);
            });
        }
    }

    void VulkanRenderer::UpdateUniformBuffer(uint32_t currentImage)
    {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        static float deltaTime = 0.0f;	// time between current frame and last frame
        static float lastFrame = 0.0f;

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        m_Camera.Update(deltaTime);

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ubo.view = m_Camera.GetViewMatrix();
        ubo.proj = glm::perspective(glm::radians(45.0f), m_Extent.width / (float) m_Extent.height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;
        
        void* data;
        vkMapMemory(m_Device, m_UniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
            memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(m_Device, m_UniformBuffersMemory[currentImage]);
    }

    void VulkanRenderer::CreateDescriptorPool()
    {
        VkDescriptorPoolSize poolSize{};
        poolSize.type               = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount    = static_cast<uint32_t>(m_Images.size());

        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(m_Images.size());
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(m_Images.size());

        VkDescriptorPoolCreateInfo poolCI{};
        poolCI.sType          = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolCI.poolSizeCount  = static_cast<uint32_t>(poolSizes.size());
        poolCI.pPoolSizes     = poolSizes.data();

        poolCI.maxSets = static_cast<uint32_t>(m_Images.size());

        auto res = vkCreateDescriptorPool(m_Device, &poolCI, nullptr, &m_DescriptorPool);
        vkh::Check(res, "Descriptor pool create failed");

        m_DeletionQueue.PushDeleter([=]()
        {
            vkDestroyDescriptorPool(m_Device, m_DescriptorPool, nullptr);
        });
    }

    void VulkanRenderer::CreateCommandBuffers()
    {
        m_CommandBuffers.resize(m_SwapchainFramebuffers.size());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType                 = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool           = m_CommandPool;
        allocInfo.level                 = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount    = static_cast<uint32_t>(m_CommandBuffers.size());

        vkh::Check(vkAllocateCommandBuffers(m_Device, &allocInfo, m_CommandBuffers.data()), "Failed to allocate command buffers!");

        for (size_t i = 0; i < m_CommandBuffers.size(); i++) 
        {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            vkh::Check(vkBeginCommandBuffer(m_CommandBuffers[i], &beginInfo), "Failed to begin recording command buffer!");

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = m_RenderPass;
            renderPassInfo.framebuffer = m_SwapchainFramebuffers[i];
            renderPassInfo.renderArea.offset = { 0, 0 };
            renderPassInfo.renderArea.extent = m_Extent;

            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color        = { 0.2f, 0.3f, 0.4f, 1.0f };
            clearValues[1].depthStencil = { 1.0f, 0 };

            renderPassInfo.clearValueCount  = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues     = clearValues.data();

            vkCmdBeginRenderPass(m_CommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
                vkCmdBindPipeline(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);

                VkBuffer vertexBuffers[]    = { m_VertexBuffer };
                VkDeviceSize offsets[]      = { 0 };
                vkCmdBindVertexBuffers(m_CommandBuffers[i], 0, 1, vertexBuffers, offsets);
                vkCmdBindIndexBuffer(m_CommandBuffers[i], m_IndexBuffer, 0, VK_INDEX_TYPE_UINT32);

                vkCmdBindDescriptorSets(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_DescriptorSets[i], 0, nullptr);

                vkCmdDrawIndexed(m_CommandBuffers[i], static_cast<uint32_t>(m_Indices.size()), 1, 0, 0, 0);
            vkCmdEndRenderPass(m_CommandBuffers[i]);

            vkh::Check(vkEndCommandBuffer(m_CommandBuffers[i]), "Failed to record command buffer!");
        }
    }

    void VulkanRenderer::CreateSyncObjects() 
    {
        m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        m_ImagesInFlight.resize(m_Images.size(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
        {
            vkh::Check(vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]));
            vkh::Check(vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]));
            vkh::Check(vkCreateFence(m_Device, &fenceInfo, nullptr, &m_InFlightFences[i]));

            m_DeletionQueue.PushDeleter([=]()
            {
                vkDestroySemaphore(m_Device, m_ImageAvailableSemaphores[i], nullptr);
                vkDestroySemaphore(m_Device, m_RenderFinishedSemaphores[i], nullptr);
                vkDestroyFence(m_Device, m_InFlightFences[i], nullptr);
            });
        }
    }

    void VulkanRenderer::LoadModel()
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t>       shapes;
        std::vector<tinyobj::material_t>    materials;
        std::string warn, err;

        bool res = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, m_ModelPath.c_str());
        ET_CORE_ASSERT(res, warn, err);

        std::unordered_map<Vertex, uint32_t> uniqueVertices;

        for (const auto& shape : shapes) 
        {
            for (const auto& index : shape.mesh.indices) 
            {
                Vertex vertex{};

                vertex.pos = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };

                vertex.color = {1.0f, 1.0f, 1.0f};

                if (uniqueVertices.count(vertex) == 0) 
                {
                    uniqueVertices[vertex] = static_cast<uint32_t>(m_Vertices.size());
                    m_Vertices.push_back(vertex);
                }

                m_Indices.push_back(uniqueVertices[vertex]);
            }
        }
    }
}