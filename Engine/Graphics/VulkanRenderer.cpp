#include "VulkanRenderer.hpp"
#include "VulkanHelper.hpp"
#include "VulkanDebugCallback.hpp"
#include "EventSystem.hpp"
#include "Vertex.hpp"

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
        CreateGraphicsPipeline();

        CreateFramebuffers();

        CreateCommandPool();
        CreateVertexBuffer();
        CreateCommandBuffers();

        CreateSyncObjects();
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

        VkSubmitInfo submitInfo{};
        submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;

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

        vkQueuePresentKHR(m_PresentQueue, &presentInfo);

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

        CreateSwapchain();
        CreateRenderPass();
        CreateGraphicsPipeline();
        CreateFramebuffers();
        CreateCommandBuffers();
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

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount    = 1;
        subpass.pColorAttachments       = &colorAttachmentRef;

        VkRenderPassCreateInfo renderPassCI{};
        renderPassCI.sType              = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCI.attachmentCount    = 1;
        renderPassCI.pAttachments       = &colorAttachment;
        renderPassCI.subpassCount       = 1;
        renderPassCI.pSubpasses         = &subpass;

        VkSubpassDependency dependency{};
        dependency.srcSubpass           = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass           = 0;
        dependency.srcStageMask         = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask        = 0;
        dependency.dstStageMask         = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask        = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        renderPassCI.dependencyCount    = 1;
        renderPassCI.pDependencies      = &dependency;

        auto res = vkCreateRenderPass(m_Device, &renderPassCI, nullptr, &m_RenderPass);
        vkh::Check(res, "Render pass create failed");

        m_DeletionQueue.PushDeleter([=]()
        {
            vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);
        });
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
        rasterizer.frontFace                = VK_FRONT_FACE_CLOCKWISE;
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
        pipelineLayoutCI.setLayoutCount           = 0; // Optional
        pipelineLayoutCI.pSetLayouts              = nullptr; // Optional
        pipelineLayoutCI.pushConstantRangeCount   = 0; // Optional
        pipelineLayoutCI.pPushConstantRanges      = nullptr; // Optional

        auto res = vkCreatePipelineLayout(m_Device, &pipelineLayoutCI, nullptr, &m_PipelineLayout);
        vkh::Check(res, "Pipeline layout crate failed");

        VkGraphicsPipelineCreateInfo pipelineCI{};
        pipelineCI.sType                  = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineCI.stageCount             = 2;
        pipelineCI.pStages                = shaderStages;
        pipelineCI.pVertexInputState      = &vertexInputCI;
        pipelineCI.pInputAssemblyState    = &inputAssembly;
        pipelineCI.pViewportState         = &viewportState;
        pipelineCI.pRasterizationState    = &rasterizer;
        pipelineCI.pMultisampleState      = &multisampling;
        pipelineCI.pDepthStencilState     = nullptr; // Optional
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
            VkImageView attachments[] = 
            {
                m_ImageViews[i]
            };

            VkFramebufferCreateInfo framebufferCI{};
            framebufferCI.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferCI.renderPass      = m_RenderPass;
            framebufferCI.attachmentCount = 1;
            framebufferCI.pAttachments    = attachments;
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

    void VulkanRenderer::CreateVertexBuffer()
    {
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        vkh::CreateBuffer(m_Device, m_GPU, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(m_Device, stagingBufferMemory, 0, bufferSize, 0, &data);
            std::memcpy(data, vertices.data(), (size_t) bufferSize);
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

            VkClearValue clearColor = { 0.2f, 0.3f, 0.4f, 1.0f };
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clearColor;

            vkCmdBeginRenderPass(m_CommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
                vkCmdBindPipeline(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);

                VkBuffer vertexBuffers[]    = { m_VertexBuffer };
                VkDeviceSize offsets[]      = { 0 };
                vkCmdBindVertexBuffers(m_CommandBuffers[i], 0, 1, vertexBuffers, offsets);

                vkCmdDraw(m_CommandBuffers[i], static_cast<uint32_t>(vertices.size()), 1, 0, 0);
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


}