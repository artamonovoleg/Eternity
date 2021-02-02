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
#include "CommandBuffer.hpp"
#include "Shader.hpp"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::string MODEL_PATH = "../models/monkey.obj";
const std::string TEXTURE_PATH = "../textures/ground.jpg";

const int MAX_FRAMES_IN_FLIGHT = 2;

struct Vertex 
{
    glm::vec3 pos;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() 
    {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

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
        Eternity::EventSystem::AddListener(EventType::WindowResizeEvent, [&](const Event& event)
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

    std::shared_ptr<Eternity::Instance>         m_Instance;
    std::shared_ptr<Eternity::Surface>          m_Surface;
    std::shared_ptr<Eternity::PhysicalDevice>   m_PhysicalDevice;
    std::shared_ptr<Eternity::Device>           m_Device;
    std::shared_ptr<Eternity::Swapchain>        m_Swapchain;

    std::shared_ptr<Eternity::DepthImage>       m_DepthImage;

    std::shared_ptr<Eternity::RenderPass>       m_RenderPass;

    std::shared_ptr<Eternity::Framebuffers>     m_Framebuffers;
    ///

    std::shared_ptr<Eternity::CommandPool>      m_CommandPool;

    std::shared_ptr<Eternity::Image2D>            m_TextureImage;

    VkDescriptorSetLayout   descriptorSetLayout;
    VkPipelineLayout        pipelineLayout;
    VkPipeline              graphicsPipeline;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    std::shared_ptr<Eternity::Buffer> m_VertexBuffer;
    std::shared_ptr<Eternity::Buffer> m_IndexBuffer;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;

    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    std::vector<std::shared_ptr<Eternity::CommandBuffer>> m_CommandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    size_t currentFrame = 0;

    bool framebufferResized = false;

    void Prepare()
    {
        m_Instance          = std::make_shared<Eternity::Instance>();
        m_Surface           = std::make_shared<Eternity::Surface>(*m_Instance);
        m_PhysicalDevice    = std::make_shared<Eternity::PhysicalDevice>(*m_Instance, *m_Surface);
        m_Device            = std::make_shared<Eternity::Device>(*m_Instance, *m_PhysicalDevice);
        m_Swapchain         = std::make_shared<Eternity::Swapchain>(ChooseSwapExtent(), *m_Device);
        
        m_DepthImage        = std::make_shared<Eternity::DepthImage>(*m_Device, m_Swapchain->GetExtent());
        CreateRenderPass();

        m_Framebuffers      = std::make_shared<Eternity::Framebuffers>(*m_Swapchain, *m_RenderPass, *m_DepthImage);
        m_CommandPool       = std::make_shared<Eternity::CommandPool>(*m_Device);
    }

    void initVulkan() 
    {
        createDescriptorSetLayout();
        createGraphicsPipeline();
        m_TextureImage = std::make_shared<Eternity::Image2D>(*m_CommandPool, TEXTURE_PATH);
        loadModel();
        createVertexBuffer(vertices.data(), sizeof(vertices[0]) * vertices.size());
        createIndexBuffer(indices.data(), sizeof(indices[0]) * indices.size());
        createUniformBuffers();
        createDescriptorPool();
        createDescriptorSets();
        createCommandBuffers();
        createSyncObjects();
    }

    void mainLoop() 
    {
        while (!Eternity::WindowShouldClose()) 
        {
            Eternity::EventSystem::PollEvents();
            drawFrame();
        }

        m_Device->WaitIdle();
    }

    void cleanupSwapChain() 
    {
        vkDestroyPipeline(*m_Device, graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(*m_Device, pipelineLayout, nullptr);

        for (size_t i = 0; i < m_Swapchain->GetImageCount(); i++) 
        {
            vkDestroyBuffer(*m_Device, uniformBuffers[i], nullptr);
            vkFreeMemory(*m_Device, uniformBuffersMemory[i], nullptr);
        }

        vkDestroyDescriptorPool(*m_Device, descriptorPool, nullptr);
    }

    void cleanup() {
        cleanupSwapChain();

        vkDestroyDescriptorSetLayout(*m_Device, descriptorSetLayout, nullptr);

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

        m_DepthImage = std::make_shared<Eternity::DepthImage>(*m_Device, m_Swapchain->GetExtent());
        CreateRenderPass();
        m_Framebuffers = std::make_shared<Eternity::Framebuffers>(*m_Swapchain, *m_RenderPass, *m_DepthImage);

        createGraphicsPipeline();
        createUniformBuffers();
        createDescriptorPool();
        createDescriptorSets();
        createCommandBuffers();
    }

    void CreateRenderPass() 
    {
        Attachment colorAttachment(Attachment::Type::Color, 0, m_Swapchain->GetImageFormat());
        Attachment depthAttachment(Attachment::Type::Depth, 1, m_DepthImage->GetFormat());

        m_RenderPass = std::make_shared<Eternity::RenderPass>(*m_Device, std::vector{ colorAttachment }, depthAttachment);
    }

    void createDescriptorSetLayout() {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.pImmutableSamplers = nullptr;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(*m_Device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    void createGraphicsPipeline() {
        
        Shader vertShader(*m_Device, Shader::Type::Vertex, "../shaders/vert.spv");
        Shader fragShader(*m_Device, Shader::Type::Vertex, "../shaders/frag.spv");
        
        ShaderStage shaderStage (vertShader, fragShader);

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

        auto bindingDescription = Vertex::getBindingDescription();
        auto attributeDescriptions = Vertex::getAttributeDescriptions();

        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float) m_Swapchain->GetExtent().width;
        viewport.height = (float) m_Swapchain->GetExtent().height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = m_Swapchain->GetExtent();

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

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

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

        if (vkCreatePipelineLayout(*m_Device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) 
            throw std::runtime_error("failed to create pipeline layout!");

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStage.GetStages();
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = *m_RenderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        if (vkCreateGraphicsPipelines(*m_Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
            throw std::runtime_error("failed to create graphics pipeline!");

    }

    void loadModel() 
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
    }

    void createVertexBuffer(const void* verticesData, uint32_t size)
    {
        VkDeviceSize bufferSize = size;

        Buffer stagingBuffer (*m_Device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        void* data;
        stagingBuffer.MapMemory(&data);
            std::memcpy(data, verticesData, (size_t) bufferSize);
        stagingBuffer.UnmapMemory();

        m_VertexBuffer = std::make_shared<Eternity::Buffer>(*m_Device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        copyBuffer(stagingBuffer, *m_VertexBuffer, bufferSize);
    }

    void createIndexBuffer(const void* indicesData, uint32_t size) 
    {
        VkDeviceSize bufferSize = size;

        Buffer stagingBuffer (*m_Device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        void* data;
        stagingBuffer.MapMemory(&data);
            std::memcpy(data, indicesData, (size_t) bufferSize);
        stagingBuffer.UnmapMemory();

        m_IndexBuffer = std::make_shared<Eternity::Buffer>(*m_Device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        
        copyBuffer(stagingBuffer, *m_IndexBuffer, bufferSize);
    }

    void createUniformBuffers() 
    {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        uniformBuffers.resize(m_Swapchain->GetImageCount());
        uniformBuffersMemory.resize(m_Swapchain->GetImageCount());

        for (size_t i = 0; i < m_Swapchain->GetImageCount(); i++) 
            createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
    }

    void createDescriptorPool() {
        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(m_Swapchain->GetImageCount());
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(m_Swapchain->GetImageCount());

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(m_Swapchain->GetImageCount());

        if (vkCreateDescriptorPool(*m_Device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    void createDescriptorSets() {
        std::vector<VkDescriptorSetLayout> layouts(m_Swapchain->GetImageCount(), descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(m_Swapchain->GetImageCount());
        allocInfo.pSetLayouts = layouts.data();

        descriptorSets.resize(m_Swapchain->GetImageCount());
        if (vkAllocateDescriptorSets(*m_Device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        for (size_t i = 0; i < m_Swapchain->GetImageCount(); i++) 
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = uniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView     = m_TextureImage->GetImageView();
            imageInfo.sampler       = m_TextureImage->GetSampler();

            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = descriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(*m_Device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) 
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(*m_Device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) 
            throw std::runtime_error("failed to create buffer!");

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(*m_Device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(m_Device->GetPhysicalDevice(), memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(*m_Device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        vkBindBufferMemory(*m_Device, buffer, bufferMemory, 0);
    }

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {

        CommandBuffer buffer = m_CommandPool->BeginSingleTimeCommands();

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(buffer, srcBuffer, dstBuffer, 1, &copyRegion);

        m_CommandPool->EndSingleTimeCommands(buffer);
    }

    void createCommandBuffers() {

        m_CommandBuffers.resize(m_Framebuffers->GetBuffersCount());
        for (int i = 0; i < m_Framebuffers->GetBuffersCount(); i++)
            m_CommandBuffers[i] = std::make_shared<Eternity::CommandBuffer>(*m_Device, *m_CommandPool);

        for (size_t i = 0; i < m_CommandBuffers.size(); i++) 
        {

            m_CommandBuffers[i]->Begin();

                VkRenderPassBeginInfo renderPassInfo{};
                renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                renderPassInfo.renderPass = *m_RenderPass;
                renderPassInfo.framebuffer = m_Framebuffers->GetBuffers().at(i);
                renderPassInfo.renderArea.offset = {0, 0};
                renderPassInfo.renderArea.extent = m_Swapchain->GetExtent();

                std::array<VkClearValue, 2> clearValues{};
                clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
                clearValues[1].depthStencil = {1.0f, 0};

                renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
                renderPassInfo.pClearValues = clearValues.data();

                m_CommandBuffers[i]->BeginRenderPass(&renderPassInfo,  VK_SUBPASS_CONTENTS_INLINE);
                    m_CommandBuffers[i]->BindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

                    VkBuffer vertexBuffers[] = { *m_VertexBuffer };
                    VkDeviceSize offsets[] = {0};

                    vkCmdBindVertexBuffers(*m_CommandBuffers[i], 0, 1, vertexBuffers, offsets);

                    vkCmdBindIndexBuffer(*m_CommandBuffers[i], *m_IndexBuffer, 0, VK_INDEX_TYPE_UINT32);

                    vkCmdBindDescriptorSets(*m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);

                    vkCmdDrawIndexed(*m_CommandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
                    
                m_CommandBuffers[i]->EndRenderPass();
            m_CommandBuffers[i]->End();
        }
    }

    void createSyncObjects() {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
        imagesInFlight.resize(m_Swapchain->GetImageCount(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(*m_Device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(*m_Device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(*m_Device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }

    void updateUniformBuffer(uint32_t currentImage) {
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), m_Swapchain->GetExtent().width / (float) m_Swapchain->GetExtent().height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;

        void* data;
        vkMapMemory(*m_Device, uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
            memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(*m_Device, uniformBuffersMemory[currentImage]);
    }

    void drawFrame() {

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

        updateUniformBuffer(m_Swapchain->GetActiveImageIndex());

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