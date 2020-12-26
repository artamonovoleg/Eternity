#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <set>
#include <fstream>
#include <istream>

#include "Base.hpp"
#include "VulkanWrapper.hpp"

namespace Eternity
{
    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR& surface)
    {
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

        if (formatCount != 0)
        {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

        if (presentModeCount != 0)
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
        for (const auto& availableFormat : availableFormats)
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return availableFormat;
        }
        return availableFormats[0];
    }

    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
    {
        for (const auto& availablePresentMode : availablePresentModes)
        {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
                return availablePresentMode;
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D ChooseSwapExtent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != UINT32_MAX)
        {
            return capabilities.currentExtent;
        }
        else
        {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            VkExtent2D actualExtent = {
                    static_cast<uint32_t>(width),
                    static_cast<uint32_t>(height)
            };

            actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

            return actualExtent;
        }
    }

    static std::vector<char> ReadShader(const std::string& shadername)
    {
        std::ifstream file(shadername, std::ios::ate | std::ios::binary);
        ET_CORE_ASSERT(file.is_open(), "Read shader file failed!");

        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        return buffer;
    }

    class VulkanRenderer
    {
        private:
            GLFWwindow*         m_Window    = nullptr;

            vkb::Instance       m_Instance            = {};
            vkb::PhysicalDevice m_PhysicalDevice      = {};
            vkb::Device         m_Device              = {};
            VkQueue             m_GraphicsQueue     = VK_NULL_HANDLE;
            VkQueue             m_PresentQueue      = VK_NULL_HANDLE;

            // TODO: MOVE IT !!!
            VkSurfaceKHR        m_Surface           = VK_NULL_HANDLE;

            VkSwapchainKHR              m_Swapchain         = VK_NULL_HANDLE;
            std::vector<VkImage>        m_SwapchainImages{};
            VkFormat                    m_SwapchainImageFormat{};
            VkExtent2D                  m_SwapchainExtent{};
            std::vector<VkImageView>    m_SwapchainImageViews{};

            VkRenderPass                m_RenderPass        = VK_NULL_HANDLE;
            // uniforms...
            VkPipelineLayout            m_PipelineLayout    = VK_NULL_HANDLE;
            VkPipeline                  m_GraphicsPipeline  = VK_NULL_HANDLE;

            std::vector<VkFramebuffer>  m_SwapchainFramebuffers{};

            VkCommandPool                   m_CommandPool       = VK_NULL_HANDLE;
            std::vector<VkCommandBuffer>    m_CommandBuffers{};

            VkSemaphore m_ImageAvailableSemaphore = VK_NULL_HANDLE;
            VkSemaphore m_RenderFinishedSemaphore = VK_NULL_HANDLE;

            void CreateInstance();
            void DestroyInstance();

            void CreateSurface();
            void DestroySurface();

            void FindPhysicalDevice();

            void CreateLogicalDevice();
            void DestroyLogicalDevice();

            void CreateSwapchain();
            void DestroySwapchain();

            void CreateImageViews();
            void DestroyImageViews();

            void CreateRenderPass();
            void DestroyRenderPass();

            void CreateGraphicsPipeline();
            void DestroyGraphicsPipeline();

            void CreateFramebuffers();
            void DestroyFramebuffers();

            void CreateCommandPool();
            void DestroyCommandPool();

            void CreateCommandBuffers();

            void CreateSemaphores();
            void DestroySemaphores();
        public:
            VulkanRenderer(GLFWwindow* window);
            ~VulkanRenderer();

            void DrawFrame();
    };

    VulkanRenderer::VulkanRenderer(GLFWwindow* window)
        : m_Window(window)
    {
        CreateInstance();
        CreateSurface();
        FindPhysicalDevice();
        CreateLogicalDevice();
        CreateSwapchain();
        CreateImageViews();
        CreateRenderPass();
        CreateGraphicsPipeline();
        CreateFramebuffers();
        CreateCommandPool();
        CreateCommandBuffers();
        CreateSemaphores();
    }

    VulkanRenderer::~VulkanRenderer()
    {
        DestroySemaphores();
        DestroyCommandPool();
        DestroyFramebuffers();
        DestroyGraphicsPipeline();
        DestroyRenderPass();
        DestroyImageViews();
        DestroySwapchain();
        DestroyLogicalDevice();
        DestroySurface();
        DestroyInstance();
    }

    void VulkanRenderer::CreateInstance()
    {
        vkb::InstanceBuilder instanceBuilder;
        instanceBuilder.SetEngineName("Eternity");
        instanceBuilder.SetEngineVersion(1, 0, 0);
        instanceBuilder.RequireAPIVersion(1, 0, 5);
        instanceBuilder.RequestDebug();
        instanceBuilder.Build();
        m_Instance = instanceBuilder.Get();
    }

    void VulkanRenderer::DestroyInstance()
    {
        m_Instance.Destroy();
    }

    // look for gpu, check founded suitability
    void VulkanRenderer::FindPhysicalDevice()
    {
        vkb::PhysicalDeviceSelector deviceSelector(m_Instance);
        deviceSelector.SetSurface(m_Surface);
        deviceSelector.Select();
        m_PhysicalDevice = deviceSelector.Get();
    }

    // create logical device
    // here we also get two queues
    void VulkanRenderer::CreateLogicalDevice()
    {
        vkb::DeviceBuilder deviceBuilder(m_Instance, m_PhysicalDevice);
        deviceBuilder.SetSurface(m_Surface);
        deviceBuilder.Build();
        m_Device = deviceBuilder.Get();

        // Get device graphic queue
        m_GraphicsQueue = m_Device.GetQueue(m_PhysicalDevice, vkb::QueueType::Graphics);
        m_PresentQueue  = m_Device.GetQueue(m_PhysicalDevice, vkb::QueueType::Presentation);
    }

    void VulkanRenderer::DestroyLogicalDevice()
    {
        m_Device.Destroy();
    }

    void VulkanRenderer::CreateSurface()
    {
        vkb::Check(glfwCreateWindowSurface(m_Instance.instance, m_Window, nullptr, &m_Surface), "Surface creation failed!");
    }

    void VulkanRenderer::DestroySurface()
    {
        vkDestroySurfaceKHR(m_Instance.instance, m_Surface, nullptr);
    }

    void VulkanRenderer::CreateSwapchain()
    {
        SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_PhysicalDevice.device, m_Surface);

        VkSurfaceFormatKHR  surfaceFormat   = ChooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR    presentMode     = ChooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D          extent          = ChooseSwapExtent(m_Window, swapChainSupport.capabilities);
        m_SwapchainImageFormat  = surfaceFormat.format;
        m_SwapchainExtent       = extent;
        // decide how many images we have in swapchain
        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
            imageCount = swapChainSupport.capabilities.maxImageCount;

        VkSwapchainCreateInfoKHR swapchainCreateInfo{};
        swapchainCreateInfo.sType               = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainCreateInfo.surface             = m_Surface;
        swapchainCreateInfo.minImageCount       = imageCount;
        swapchainCreateInfo.imageFormat         = surfaceFormat.format;
        swapchainCreateInfo.imageColorSpace     = surfaceFormat.colorSpace;
        swapchainCreateInfo.imageExtent         = extent;
        swapchainCreateInfo.imageArrayLayers    = 1;
        swapchainCreateInfo.imageUsage          = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        uint32_t queueFamilyIndices[] = { m_Device.GetQueueFamilyIndex(m_PhysicalDevice, vkb::QueueType::Graphics),
                                          m_Device.GetQueueFamilyIndex(m_PhysicalDevice, vkb::QueueType::Presentation) };

        if (m_Device.GetQueueFamilyIndex(m_PhysicalDevice, vkb::QueueType::Graphics) != m_Device.GetQueueFamilyIndex(m_PhysicalDevice, vkb::QueueType::Presentation))
        {
            swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapchainCreateInfo.queueFamilyIndexCount = 2;
            swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            swapchainCreateInfo.queueFamilyIndexCount = 0; // Optional
            swapchainCreateInfo.pQueueFamilyIndices = nullptr; // Optional
        }
        swapchainCreateInfo.preTransform    = swapChainSupport.capabilities.currentTransform;
        swapchainCreateInfo.compositeAlpha  = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchainCreateInfo.presentMode     = presentMode;
        swapchainCreateInfo.clipped         = VK_TRUE;
        swapchainCreateInfo.oldSwapchain    = VK_NULL_HANDLE;

        ET_CORE_ASSERT(vkCreateSwapchainKHR(m_Device.device, &swapchainCreateInfo, nullptr, &m_Swapchain) == VK_SUCCESS, "Swapchain creation");

        vkGetSwapchainImagesKHR(m_Device.device, m_Swapchain, &imageCount, nullptr);
        m_SwapchainImages.resize(imageCount);
        ET_CORE_ASSERT(vkGetSwapchainImagesKHR(m_Device.device, m_Swapchain, &imageCount, m_SwapchainImages.data()) == VK_SUCCESS, "Get swapchain images");
    }

    void VulkanRenderer::DestroySwapchain()
    {
        vkDestroySwapchainKHR(m_Device.device, m_Swapchain, nullptr);
    }

    void VulkanRenderer::CreateImageViews()
    {
        m_SwapchainImageViews.resize(m_SwapchainImages.size());
        for (size_t i = 0; i < m_SwapchainImages.size(); i++)
        {
            VkImageViewCreateInfo imageViewCreateInfo{};
            imageViewCreateInfo.sType        = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewCreateInfo.image        = m_SwapchainImages[i];
            imageViewCreateInfo.viewType     = VK_IMAGE_VIEW_TYPE_2D;
            imageViewCreateInfo.format       = m_SwapchainImageFormat;
            imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            imageViewCreateInfo.subresourceRange.baseMipLevel   = 0;
            imageViewCreateInfo.subresourceRange.levelCount     = 1;
            imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
            imageViewCreateInfo.subresourceRange.layerCount     = 1;

            ET_CORE_ASSERT(vkCreateImageView(m_Device.device, &imageViewCreateInfo, nullptr, &m_SwapchainImageViews[i]) == VK_SUCCESS, "Failed to create image view");
        }
    }

    void VulkanRenderer::DestroyImageViews()
    {
        for (const auto& imageView : m_SwapchainImageViews)
        {
            vkDestroyImageView(m_Device.device, imageView, nullptr);
        }
    }

    VkShaderModule CreateShaderModule(const VkDevice& device, const std::vector<char>& code)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
        VkShaderModule shaderModule;
        ET_CORE_ASSERT(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) == VK_SUCCESS);
        return shaderModule;
    }

    void VulkanRenderer::CreateRenderPass()
    {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format          = m_SwapchainImageFormat;
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

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType            = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount  = 1;
        renderPassInfo.pAttachments     = &colorAttachment;
        renderPassInfo.subpassCount     = 1;
        renderPassInfo.pSubpasses       = &subpass;


        VkSubpassDependency dependency{};
        dependency.srcSubpass       = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass       = 0;
        dependency.srcStageMask     = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask    = 0;
        dependency.dstStageMask     = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask    = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        ET_CORE_ASSERT(vkCreateRenderPass(m_Device.device, &renderPassInfo, nullptr, &m_RenderPass) == VK_SUCCESS, "Create render pass failed!");

    }

    void VulkanRenderer::DestroyRenderPass()
    {
        vkDestroyRenderPass(m_Device.device, m_RenderPass, nullptr);
    }

    void VulkanRenderer::CreateGraphicsPipeline()
    {
        auto vertShaderCode = ReadShader("shaders/vert.spv");
        auto fragShaderCode = ReadShader("shaders/frag.spv");

        VkShaderModule vertShaderModule = CreateShaderModule(m_Device.device, vertShaderCode);
        VkShaderModule fragShaderModule = CreateShaderModule(m_Device.device, fragShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";


        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };
        // we hardcode all in shader. because that all is empty TODO: load vertex from array
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
        vertexInputInfo.vertexAttributeDescriptionCount = 0;
        vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

        // say how we will draw data
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType                     = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology                  = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable    = VK_FALSE;

        // Create viewport))))
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float) m_SwapchainExtent.width;
        viewport.height = (float) m_SwapchainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = m_SwapchainExtent;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType                        = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable             = VK_FALSE;
        rasterizer.rasterizerDiscardEnable      = VK_FALSE;
        rasterizer.polygonMode                  = VK_POLYGON_MODE_FILL; // We can set to warframe or something else
        rasterizer.lineWidth                    = 1.0f;
        rasterizer.cullMode                     = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace                    = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable              = VK_FALSE;
        rasterizer.depthBiasConstantFactor      = 0.0f; // Optional
        rasterizer.depthBiasClamp               = 0.0f; // Optional
        rasterizer.depthBiasSlopeFactor         = 0.0f; // Optional

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType                     = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable       = VK_FALSE;
        multisampling.rasterizationSamples      = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading          = 1.0f; // Optional
        multisampling.pSampleMask               = nullptr; // Optional
        multisampling.alphaToCoverageEnable     = VK_FALSE; // Optional
        multisampling.alphaToOneEnable          = VK_FALSE; // Optional

        // when we have more when one framebuffer we must say vulkan how to blend new values and old & or mix
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
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f; // Optional
        colorBlending.blendConstants[1] = 0.0f; // Optional
        colorBlending.blendConstants[2] = 0.0f; // Optional
        colorBlending.blendConstants[3] = 0.0f; // Optional

        // this uses for uniforms
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType                    = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount           = 0; // Optional
        pipelineLayoutInfo.pSetLayouts              = nullptr; // Optional
        pipelineLayoutInfo.pushConstantRangeCount   = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges      = nullptr; // Optional

        ET_CORE_ASSERT(vkCreatePipelineLayout(m_Device.device, &pipelineLayoutInfo, nullptr, &m_PipelineLayout) == VK_SUCCESS, "Create pipeline layout");

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType                  = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount             = 2;
        pipelineInfo.pStages                = shaderStages;
        pipelineInfo.pVertexInputState      = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState    = &inputAssembly;
        pipelineInfo.pViewportState         = &viewportState;
        pipelineInfo.pRasterizationState    = &rasterizer;
        pipelineInfo.pMultisampleState      = &multisampling;
        pipelineInfo.pDepthStencilState     = nullptr; // Optional
        pipelineInfo.pColorBlendState       = &colorBlending;
        pipelineInfo.pDynamicState          = nullptr; // Optional
        pipelineInfo.layout                 = m_PipelineLayout;
        pipelineInfo.renderPass             = m_RenderPass;
        pipelineInfo.subpass                = 0;
        pipelineInfo.basePipelineHandle     = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex      = -1; // Optional

        ET_CORE_ASSERT(vkCreateGraphicsPipelines(m_Device.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline) == VK_SUCCESS, "Create pipeline");

        // cleanup. dont need shader modules more
        vkDestroyShaderModule(m_Device.device, vertShaderModule, nullptr);
        vkDestroyShaderModule(m_Device.device, fragShaderModule, nullptr);
    }

    void VulkanRenderer::DestroyGraphicsPipeline()
    {
        vkDestroyPipeline(m_Device.device, m_GraphicsPipeline, nullptr);
        vkDestroyPipelineLayout(m_Device.device, m_PipelineLayout, nullptr);
    }

    void VulkanRenderer::CreateFramebuffers()
    {
        m_SwapchainFramebuffers.resize(m_SwapchainImageViews.size());

        for (size_t i = 0; i < m_SwapchainImageViews.size(); i++)
        {
            VkImageView attachments[] = { m_SwapchainImageViews[i] };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass      = m_RenderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments    = attachments;
            framebufferInfo.width           = m_SwapchainExtent.width;
            framebufferInfo.height          = m_SwapchainExtent.height;
            framebufferInfo.layers          = 1;

            ET_CORE_ASSERT(vkCreateFramebuffer(m_Device.device, &framebufferInfo, nullptr, &m_SwapchainFramebuffers[i]) == VK_SUCCESS);
        }
    }

    void VulkanRenderer::DestroyFramebuffers()
    {
        for (auto framebuffer : m_SwapchainFramebuffers)
            vkDestroyFramebuffer(m_Device.device, framebuffer, nullptr);
    }

    void VulkanRenderer::CreateCommandPool()
    {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = m_Device.GetQueueFamilyIndex(m_PhysicalDevice, vkb::QueueType::Presentation);
        poolInfo.flags = 0; // Optional

        vkb::Check(vkCreateCommandPool(m_Device.device, &poolInfo, nullptr, &m_CommandPool), "Command pool cration failed!");
    }

    void VulkanRenderer::DestroyCommandPool()
    {
        vkDestroyCommandPool(m_Device.device, m_CommandPool, nullptr);
    }

    void VulkanRenderer::CreateCommandBuffers()
    {
        m_CommandBuffers.resize(m_SwapchainFramebuffers.size());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_CommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());

        ET_CORE_ASSERT(vkAllocateCommandBuffers(m_Device.device, &allocInfo, m_CommandBuffers.data()) == VK_SUCCESS, "Allocate command buffers failed!");

        for (size_t i = 0; i < m_CommandBuffers.size(); i++)
        {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = 0; // Optional
            beginInfo.pInheritanceInfo = nullptr; // Optional

            ET_CORE_ASSERT(vkBeginCommandBuffer(m_CommandBuffers[i], &beginInfo) == VK_SUCCESS, "Failed to begin recording command buffer!")

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType        = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass   = m_RenderPass;
            renderPassInfo.framebuffer  = m_SwapchainFramebuffers[i];
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = m_SwapchainExtent;

            VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clearColor;

            vkCmdBeginRenderPass(m_CommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);
            vkCmdDraw(m_CommandBuffers[i], 3, 1, 0, 0);
            vkCmdEndRenderPass(m_CommandBuffers[i]);

            ET_CORE_ASSERT(vkEndCommandBuffer(m_CommandBuffers[i]) == VK_SUCCESS, "Failed to record cmdb");
        }
    }

    void VulkanRenderer::CreateSemaphores()
    {
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        ET_CORE_ASSERT(vkCreateSemaphore(m_Device.device, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphore) == VK_SUCCESS &&
        vkCreateSemaphore(m_Device.device, &semaphoreInfo, nullptr, &m_RenderFinishedSemaphore) == VK_SUCCESS);
    }

    void VulkanRenderer::DestroySemaphores()
    {
        vkDestroySemaphore(m_Device.device, m_RenderFinishedSemaphore, nullptr);
        vkDestroySemaphore(m_Device.device, m_ImageAvailableSemaphore, nullptr);
    }

    void VulkanRenderer::DrawFrame()
    {
        uint32_t imageIndex;
        vkAcquireNextImageKHR(m_Device.device, m_Swapchain, UINT64_MAX, m_ImageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphore };
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_CommandBuffers[imageIndex];

        VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphore };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        ET_CORE_ASSERT(vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) == VK_SUCCESS, "Failed to submit draw command buffer!");

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { m_Swapchain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;

        presentInfo.pResults = nullptr; // Optional
        vkQueuePresentKHR(m_PresentQueue, &presentInfo);
        vkQueueWaitIdle(m_PresentQueue);
    }
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "title", nullptr, nullptr);

    Eternity::VulkanRenderer renderer(window);
    while (!glfwWindowShouldClose(window))
    {
        renderer.DrawFrame();
        glfwPollEvents();
    }

    glfwTerminate();
    return EXIT_SUCCESS;
}