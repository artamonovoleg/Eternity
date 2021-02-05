#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <iostream>
#include <stdexcept>
#include <memory>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <array>
#include <unordered_map>
#include "Utils.hpp"
#include "Window.hpp"
#include "EventSystem.hpp"
#include "Input.hpp"
#include "Instance.hpp"
#include "Surface.hpp"
#include "PhysicalDevice.hpp"
#include "Device.hpp"
#include "Swapchain.hpp"
#include "RenderPass.hpp"
#include "Image.hpp"
#include "DepthImage.hpp"
#include "Image2D.hpp"
#include "Framebuffers.hpp"
#include "CommandPool.hpp"
#include "Buffer.hpp"
#include "UniformBuffer.hpp"
#include "CommandBuffer.hpp"
#include "Shader.hpp"
#include "Descriptors.hpp"
#include "DescriptorPool.hpp"
#include "DescriptorSets.hpp"
#include "GraphicsPipelineLayout.hpp"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::string MODEL_PATH = "../models/monkey.obj";
const std::string TEXTURE_PATH = "../textures/ground.jpg";

const int MAX_FRAMES_IN_FLIGHT = 2;

struct Vertex 
{
    glm::vec3 pos;
    glm::vec2 texCoord;

    static std::vector<VkVertexInputBindingDescription> getBindingDescription() 
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return std::vector<VkVertexInputBindingDescription>{bindingDescription};
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() 
    {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);

        attributeDescriptions[0] = {};
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);
        attributeDescriptions[1] = {};
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }

    bool operator==(const Vertex& other) const 
    {
        return pos == other.pos && texCoord == other.texCoord;
    }
};

namespace std 
{
    template<> struct hash<Vertex> 
    {
        size_t operator()(Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}

struct UniformBufferObject 
{
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

using namespace Eternity;

class Renderable
{
    public:
        std::vector<Vertex>                             vertices;
        std::vector<uint32_t>                           indices;

        std::shared_ptr<Buffer>                         m_VertexBuffer;
        std::shared_ptr<Buffer>                         m_IndexBuffer;

        std::vector<std::shared_ptr<UniformBuffer>>     m_UniformBuffers;
};

class VulkanApp 
{
public:
    void run() 
    {
        Prepare();
        initVulkan();
        mainLoop();
        cleanup();
    }

    VulkanApp()
    {
        // later delete this
        EventSystem::AddListener(EventType::WindowResizeEvent, [&](const Event& event)
        {
            framebufferResized = true;
            int width = 0, height = 0;
            glfwGetFramebufferSize(Eternity::GetWindow(), &width, &height);
            while (width == 0 || height == 0)
            {
                glfwGetFramebufferSize(Eternity::GetWindow(), &width, &height);
                glfwWaitEvents();
            }
        });
    }

private:

    /// Rewrited code
    VkExtent2D ChooseSwapExtent();

    std::shared_ptr<Instance>                       m_Instance;
    std::shared_ptr<Surface>                        m_Surface;
    std::shared_ptr<PhysicalDevice>                 m_PhysicalDevice;
    std::shared_ptr<Device>                         m_Device;
    std::shared_ptr<Swapchain>                      m_Swapchain;
    std::shared_ptr<DepthImage>                     m_DepthImage;
    std::shared_ptr<RenderPass>                     m_RenderPass;
    std::shared_ptr<Framebuffers>                   m_Framebuffers;
    std::shared_ptr<CommandPool>                    m_CommandPool;
    std::shared_ptr<Image2D>                        m_TextureImage;
    std::shared_ptr<DescriptorSetLayout>            m_DescriptorSetLayout;
    ///

    std::shared_ptr<GraphicsPipelineLayout>         m_PipelineLayout;
    VkPipeline              graphicsPipeline;

    std::vector<Vertex>                             vertices;
    std::vector<uint32_t>                           indices;

    std::vector<Renderable>                         m_Renderables;
    std::vector<std::shared_ptr<UniformBuffer>>     m_UniformBuffers;

    std::shared_ptr<DescriptorPool>                 m_DescriptorPool;
    std::shared_ptr<DescriptorSets>                 m_DescriptorSets;
    std::vector<std::shared_ptr<CommandBuffer>>     m_CommandBuffers;

    std::vector<VkSemaphore>    imageAvailableSemaphores;
    std::vector<VkSemaphore>    renderFinishedSemaphores;
    std::vector<VkFence>        inFlightFences;
    std::vector<VkFence>        imagesInFlight;
    size_t currentFrame = 0;

    bool framebufferResized = false;

    void Prepare()
    {
        m_Instance          = std::make_shared<Instance>();
        m_Surface           = std::make_shared<Surface>(*m_Instance);
        m_PhysicalDevice    = std::make_shared<PhysicalDevice>(*m_Instance, *m_Surface);
        m_Device            = std::make_shared<Device>(*m_Instance, *m_PhysicalDevice);
        m_Swapchain         = std::make_shared<Swapchain>(ChooseSwapExtent(), *m_Device);
        
        m_DepthImage        = std::make_shared<DepthImage>(*m_Device, m_Swapchain->GetExtent());

        CreateRenderPass();

        m_Framebuffers      = std::make_shared<Framebuffers>(*m_Swapchain, *m_RenderPass, *m_DepthImage);
        m_CommandPool       = std::make_shared<CommandPool>(*m_Device);

        CreateDescriptorSetLayout();

        m_TextureImage = std::make_shared<Image2D>(*m_CommandPool, TEXTURE_PATH);

        CreateSyncObjects();
    }

    void initVulkan() 
    {
        createGraphicsPipeline();
        
        LoadModel();
        CreateUniformBuffers();
        
        CreateDescriptorPool();
        CreateDescriptorSets();
        CreateCommandBuffers();
    }

    void mainLoop() 
    {
        while (!Eternity::WindowShouldClose()) 
        {
            EventSystem::PollEvents();
            drawFrame();
        }

        m_Device->WaitIdle();
    }

    void cleanupSwapChain() 
    {
        vkDestroyPipeline(*m_Device, graphicsPipeline, nullptr);
    }

    void cleanup() {
        cleanupSwapChain();

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroySemaphore(*m_Device, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(*m_Device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(*m_Device, inFlightFences[i], nullptr);
        }
    }

    void recreateSwapChain() 
    {
        m_Device->WaitIdle();

        cleanupSwapChain();

        m_Swapchain->Recreate(ChooseSwapExtent());

        m_DepthImage    = std::make_shared<DepthImage>(*m_Device, m_Swapchain->GetExtent());
        CreateRenderPass();
        m_Framebuffers  = std::make_shared<Framebuffers>(*m_Swapchain, *m_RenderPass, *m_DepthImage);

        createGraphicsPipeline();
        CreateUniformBuffers();
        CreateDescriptorPool();
        CreateDescriptorSets();
        CreateCommandBuffers();
    }

    void CreateRenderPass() 
    {
        Attachment colorAttachment(Attachment::Type::Color, 0, m_Swapchain->GetImageFormat());
        Attachment depthAttachment(Attachment::Type::Depth, 1, m_DepthImage->GetFormat());

        m_RenderPass = std::make_shared<RenderPass>(*m_Device, std::vector{ colorAttachment }, depthAttachment);
    }

    void CreateDescriptorSetLayout() 
    {
        VkDescriptorSetLayoutBinding uboLayoutBinding       = UniformBuffer::GetDescriptorSetLayout(0, 1);
        VkDescriptorSetLayoutBinding samplerLayoutBinding   = Image2D::GetDescriptorSetLayout(1, 1);

        std::vector<VkDescriptorSetLayoutBinding> bindings = { uboLayoutBinding, samplerLayoutBinding };
        m_DescriptorSetLayout = std::make_shared<DescriptorSetLayout>(*m_Device, bindings);
    }

    void createGraphicsPipeline() 
    {
        m_PipelineLayout = std::make_shared<GraphicsPipelineLayout>(*m_Device, *m_DescriptorSetLayout);

        Shader vertShader(*m_Device, Shader::Type::Vertex, "../shaders/vert.spv");
        Shader fragShader(*m_Device, Shader::Type::Vertex, "../shaders/frag.spv");
        
        ShaderStage shaderStage (vertShader, fragShader);

        auto bindingDescriptions   = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        VertexInput vertexInput(bindingDescriptions, attributeDescriptions);

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType     = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology  = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport{};
        viewport.x          = 0.0f;
        viewport.y          = 0.0f;
        viewport.width      = (float) m_Swapchain->GetExtent().width;
        viewport.height     = (float) m_Swapchain->GetExtent().height;
        viewport.minDepth   = 0.0f;
        viewport.maxDepth   = 1.0f;

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = m_Swapchain->GetExtent();

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
        pipelineInfo.layout                 = *m_PipelineLayout;
        pipelineInfo.renderPass             = *m_RenderPass;
        pipelineInfo.subpass                = 0;
        pipelineInfo.basePipelineHandle     = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(*m_Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
            throw std::runtime_error("failed to create graphics pipeline!");

    }

    void LoadModel() 
    {
        static bool once = true;

        if (once)
        {
            tinyobj::attrib_t attrib;
            std::vector<tinyobj::shape_t> shapes;
            std::vector<tinyobj::material_t> materials;
            std::string warn, err;

            if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str())) 
            {
                throw std::runtime_error(warn + err);
            }

            std::unordered_map<Vertex, uint32_t> uniqueVertices{};

            for (const auto& shape : shapes) {
                for (const auto& index : shape.mesh.indices) {
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

                    if (uniqueVertices.count(vertex) == 0) 
                    {
                        uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                        vertices.push_back(vertex);
                    }

                    indices.push_back(uniqueVertices[vertex]);
                }
            }
            once = false;
        }

        m_Renderables.resize(m_Renderables.size() + 1);
        m_Renderables.back().m_VertexBuffer = CreateVertexBuffer(*m_CommandPool, vertices.data(), sizeof(vertices[0]) * vertices.size());
        m_Renderables.back().m_IndexBuffer  = CreateIndexBuffer(*m_CommandPool, indices.data(), sizeof(indices[0]) * indices.size());

        for (auto& i : vertices)
            i.pos += glm::vec3(0.0f, 1.0f, 0.0f);
    }

    void CreateUniformBuffers() 
    {
        m_UniformBuffers.resize(m_Swapchain->GetImageCount());

        for (size_t i = 0; i < m_UniformBuffers.size(); i++) 
            m_UniformBuffers[i] = std::make_shared<UniformBuffer>(*m_Device, sizeof(UniformBufferObject));
    }

    void CreateDescriptorPool() 
    {
        const std::vector<DescriptorType> descriptorTypes { DescriptorType::Uniform, DescriptorType::ImageSampler };
        m_DescriptorPool = std::make_shared<DescriptorPool>(*m_Swapchain, descriptorTypes);
    }

    void CreateDescriptorSets() 
    {
        m_DescriptorSets = std::make_shared<DescriptorSets>(*m_Swapchain, *m_DescriptorSetLayout, *m_DescriptorPool);

        for (size_t i = 0; i < m_Swapchain->GetImageCount(); i++) 
        {
            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

            descriptorWrites[0]         = m_UniformBuffers[i]->GetWriteDescriptorSet(0, 1, sizeof(UniformBufferObject));
            descriptorWrites[0].dstSet  = m_DescriptorSets->GetSet(i);

            descriptorWrites[1]         = m_TextureImage->GetWriteDescriptorSet(1, 1);
            descriptorWrites[1].dstSet  = m_DescriptorSets->GetSet(i);

            m_DescriptorSets->UpdateSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data());
        }
    }

    void CreateCommandBuffers() 
    {
        m_CommandBuffers.resize(m_Framebuffers->GetBuffersCount());
        for (int i = 0; i < m_Framebuffers->GetBuffersCount(); i++)
            m_CommandBuffers[i] = std::make_shared<CommandBuffer>(*m_Device, *m_CommandPool);

        for (size_t i = 0; i < m_CommandBuffers.size(); i++) 
        {
            m_CommandBuffers[i]->Begin();

                VkRenderPassBeginInfo renderPassInfo{};
                renderPassInfo.sType                = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                renderPassInfo.renderPass           = *m_RenderPass;
                renderPassInfo.framebuffer          = m_Framebuffers->GetBuffers().at(i);
                renderPassInfo.renderArea.offset    = { 0, 0 };
                renderPassInfo.renderArea.extent    = m_Swapchain->GetExtent();

                std::array<VkClearValue, 2> clearValues{};
                clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
                clearValues[1].depthStencil = { 1, 0 };

                renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
                renderPassInfo.pClearValues = clearValues.data();

                m_CommandBuffers[i]->BeginRenderPass(&renderPassInfo,  VK_SUBPASS_CONTENTS_INLINE);
                    m_CommandBuffers[i]->BindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

                    for (const auto& renderable : m_Renderables)
                    {
                        VkBuffer vertexBuffers[] = { *renderable.m_VertexBuffer };
                        VkDeviceSize offsets[] = { 0 };

                        vkCmdBindVertexBuffers(*m_CommandBuffers[i], 0, 1, vertexBuffers, offsets);

                        vkCmdBindIndexBuffer(*m_CommandBuffers[i], *renderable.m_IndexBuffer, 0, VK_INDEX_TYPE_UINT32);

                        vkCmdBindDescriptorSets(*m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, *m_PipelineLayout, 0, 1, &m_DescriptorSets->GetSet(i), 0, nullptr);

                        vkCmdDrawIndexed(*m_CommandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
                    }
                m_CommandBuffers[i]->EndRenderPass();
            m_CommandBuffers[i]->End();
        }
    }

    void CreateSyncObjects() 
    {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        imagesInFlight.resize(m_Swapchain->GetImageCount(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
        {
            if (vkCreateSemaphore(*m_Device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(*m_Device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(*m_Device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) 
            {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }

    void UpdateUniformBuffer(uint32_t currentImage) 
    {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), m_Swapchain->GetExtent().width / (float) m_Swapchain->GetExtent().height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;

        void* data;
        m_UniformBuffers[currentImage]->MapMemory(sizeof(ubo), &data);
            std::memcpy(data, &ubo, sizeof(ubo));
        m_UniformBuffers[currentImage]->UnmapMemory();
    }

    void drawFrame() {

        if (Input::GetKeyDown(Key::A))
        {
            m_Device->WaitIdle();
            m_Renderables.pop_back();
            for (auto& i : vertices)
                i.pos -= glm::vec3(0.0f, 1.0f, 0.0f);
            CreateCommandBuffers();
        }
        else
        if (Input::GetKeyDown(Key::D))
        {
            m_Device->WaitIdle();
            LoadModel();
            CreateCommandBuffers();
        }

        VkResult result = m_Swapchain->AcquireNextImage(imageAvailableSemaphores[currentFrame], inFlightFences[currentFrame]);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) 
        {
            recreateSwapChain();
            return;
        } 
        else 
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) 
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        UpdateUniformBuffer(m_Swapchain->GetActiveImageIndex());

        if (imagesInFlight[m_Swapchain->GetActiveImageIndex()] != VK_NULL_HANDLE) 
            vkWaitForFences(*m_Device, 1, &imagesInFlight[m_Swapchain->GetActiveImageIndex()], VK_TRUE, UINT64_MAX);

        imagesInFlight[m_Swapchain->GetActiveImageIndex()] = inFlightFences[currentFrame];

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        const VkCommandBuffer& cmdBuff = *m_CommandBuffers[m_Swapchain->GetActiveImageIndex()];
        submitInfo.pCommandBuffers = &cmdBuff;

        VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkResetFences(*m_Device, 1, &inFlightFences[currentFrame]);

        if (vkQueueSubmit(m_Device->GetQueue(QueueType::Present), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) 
            throw std::runtime_error("failed to submit draw command buffer!");

        result = m_Swapchain->QueuePresent(m_Device->GetQueue(QueueType::Present), renderFinishedSemaphores[currentFrame]);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) 
        {
            framebufferResized = false;
            recreateSwapChain();
        } 
        else 
        if (result != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to present swap chain image!");
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }
};

VkExtent2D VulkanApp::ChooseSwapExtent()
{
    VkSurfaceCapabilitiesKHR capabilities {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(*m_PhysicalDevice, *m_Surface, &capabilities);

    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
    {
        return capabilities.currentExtent;
    } 
    else 
    {
        int width, height;
        glfwGetFramebufferSize(Eternity::GetWindow(), &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width  = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}

int main() 
{
    Eternity::CreateWindow(800, 600, "Eternity");
    Eternity::EventSystem::Init();
    Eternity::Input::Init();

    VulkanApp app;
    app.run();

    Eternity::DestroyWindow();

    return EXIT_SUCCESS;
}