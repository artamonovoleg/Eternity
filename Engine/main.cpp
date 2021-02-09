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
#include <list>
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
#include "GraphicsPipeline.hpp"

#include "Camera.hpp"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::string MODEL_PATH = "../models/monkey.obj";
const std::string TEXTURE_PATH = "../textures/atlas.png";

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

struct UBOMatrices 
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
    private:
        VkExtent2D ChooseSwapExtent(uint32_t width, uint32_t height);

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

        std::shared_ptr<GraphicsPipelineLayout>         m_PipelineLayout;
        std::shared_ptr<GraphicsPipeline>               m_GraphicsPipeline;

        std::vector<std::shared_ptr<Buffer>>            m_VertexBuffers;
        std::vector<std::shared_ptr<Buffer>>            m_IndexBuffers;
        std::vector<std::shared_ptr<UniformBuffer>>     m_UniformBuffers;

        std::shared_ptr<DescriptorPool>                 m_DescriptorPool;
        std::shared_ptr<DescriptorSets>                 m_DescriptorSets;
        std::vector<std::shared_ptr<CommandBuffer>>     m_CommandBuffers;

        std::vector<VkSemaphore>    imageAvailableSemaphores;
        std::vector<VkSemaphore>    renderFinishedSemaphores;
        std::vector<VkFence>        inFlightFences;
        std::vector<VkFence>        imagesInFlight;
        size_t currentFrame = 0;


        // ------------------------------------------------------------------------------//
        std::shared_ptr<Camera>                         m_RenderCamera;
        // ------------------------------------------------------------------------------//

        void Prepare()
        {
            m_Instance          = std::make_shared<Instance>();
            m_Surface           = std::make_shared<Surface>(*m_Instance);
            m_PhysicalDevice    = std::make_shared<PhysicalDevice>(*m_Instance, *m_Surface);
            m_Device            = std::make_shared<Device>(*m_Instance, *m_PhysicalDevice);
            m_Swapchain         = std::make_shared<Swapchain>(ChooseSwapExtent(Eternity::GetWindowWidth(), Eternity::GetWindowHeight()), *m_Device);
            
            m_DepthImage        = std::make_shared<DepthImage>(*m_Device, m_Swapchain->GetExtent());

            CreateRenderPass();

            m_Framebuffers      = std::make_shared<Framebuffers>(*m_Swapchain, *m_RenderPass, *m_DepthImage);
            m_CommandPool       = std::make_shared<CommandPool>(*m_Device);

            CreateDescriptorSetLayout();

            m_TextureImage = std::make_shared<Image2D>(*m_CommandPool, TEXTURE_PATH);

            CreateSyncObjects();

            CreateGraphicsPipeline();
            
            CreateUniformBuffers();
            
            CreateDescriptorPool();
            CreateDescriptorSets();
            CreateCommandBuffers();
        }

        void Cleanup() 
        {
            m_Device->WaitIdle();
            for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
            {
                vkDestroySemaphore(*m_Device, renderFinishedSemaphores[i], nullptr);
                vkDestroySemaphore(*m_Device, imageAvailableSemaphores[i], nullptr);
                vkDestroyFence(*m_Device, inFlightFences[i], nullptr);
            }
        }

        void RecreateSwapchain(const VkExtent2D& extent)
        {
            m_Device->WaitIdle();

            m_Swapchain->Recreate(extent);

            m_DepthImage    = std::make_shared<DepthImage>(*m_Device, m_Swapchain->GetExtent());
            CreateRenderPass();
            m_Framebuffers  = std::make_shared<Framebuffers>(*m_Swapchain, *m_RenderPass, *m_DepthImage);

            CreateGraphicsPipeline();
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

        void CreateGraphicsPipeline() 
        {
            m_PipelineLayout = std::make_shared<GraphicsPipelineLayout>(*m_Device, *m_DescriptorSetLayout);

            Shader vertShader(*m_Device, Shader::Type::Vertex, "../shaders/vert.spv");
            Shader fragShader(*m_Device, Shader::Type::Vertex, "../shaders/frag.spv");
            
            ShaderStage shaderStage (vertShader, fragShader);

            auto bindingDescriptions   = Vertex::getBindingDescription();
            auto attributeDescriptions = Vertex::getAttributeDescriptions();

            VertexInput vertexInput(bindingDescriptions, attributeDescriptions);

            m_GraphicsPipeline = std::make_shared<GraphicsPipeline>(*m_Device, *m_RenderPass, shaderStage, vertexInput, *m_PipelineLayout, m_Swapchain->GetExtent());
        }

    private:
        void CreateUniformBuffers() 
        {
            m_UniformBuffers.resize(m_Swapchain->GetImageCount());

            for (size_t i = 0; i < m_UniformBuffers.size(); i++) 
                m_UniformBuffers[i] = std::make_shared<UniformBuffer>(*m_Device, sizeof(UBOMatrices));
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

                descriptorWrites[0]         = m_UniformBuffers[i]->GetWriteDescriptorSet(0, 1, sizeof(UBOMatrices));
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
                        m_CommandBuffers[i]->BindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, *m_GraphicsPipeline);

                        for (size_t j = 0; j < m_VertexBuffers.size(); j++)
                        {
                            VkBuffer vertexBuffers[] = { *m_VertexBuffers[j] };
                            VkDeviceSize offsets[] = { 0 };

                            vkCmdBindVertexBuffers(*m_CommandBuffers[i], 0, 1, vertexBuffers, offsets);

                            vkCmdBindIndexBuffer(*m_CommandBuffers[i], *m_IndexBuffers[j], 0, VK_INDEX_TYPE_UINT32);

                            vkCmdBindDescriptorSets(*m_CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, *m_PipelineLayout, 0, 1, &m_DescriptorSets->GetSet(i), 0, nullptr);

                            vkCmdDrawIndexed(*m_CommandBuffers[i], static_cast<uint32_t>(m_IndexBuffers[j]->GetSize() / sizeof(uint32_t)), 1, 0, 0, 0);
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

            UBOMatrices ubo{};
            ubo.model = glm::mat4(1.0f);
            if (m_RenderCamera != nullptr)
                ubo.view = m_RenderCamera->GetViewMatrix();
            else
                ubo.view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f));
            ubo.proj = glm::perspective(glm::radians(45.0f), m_Swapchain->GetExtent().width / (float) m_Swapchain->GetExtent().height, 0.1f, 30.0f);
            ubo.proj[1][1] *= -1;

            void* data;
            m_UniformBuffers[currentImage]->MapMemory(sizeof(ubo), &data);
                std::memcpy(data, &ubo, sizeof(ubo));
            m_UniformBuffers[currentImage]->UnmapMemory();
        }
    public:
        VulkanApp()
        {
            // later delete this
            EventSystem::AddListener(EventType::WindowResizeEvent, [&](const Event& event)
            {
                auto& windowSize = static_cast<const WindowResizeEvent&>(event).GetSize();
                while (windowSize.width == 0 || windowSize.height == 0)
                    glfwWaitEvents();
                RecreateSwapchain(ChooseSwapExtent(windowSize.width, windowSize.height));
            });

            Prepare();
        }

        ~VulkanApp()
        {
            Cleanup();
        }
        
        void SetRenderCamera(std::shared_ptr<Camera>& camera)
        {
            m_RenderCamera = camera;
        }

        void LoadModel(Renderable& model, size_t& bind) 
        {
            m_Device->WaitIdle();
            
            m_VertexBuffers.emplace_back(CreateVertexBuffer(*m_CommandPool, model.vertices.data(), sizeof(model.vertices[0]) * model.vertices.size()));
            m_IndexBuffers.emplace_back(CreateIndexBuffer(*m_CommandPool, model.indices.data(), sizeof(model.indices[0]) * model.indices.size()));

            CreateCommandBuffers();

            bind = m_VertexBuffers.size() - 1;
        }

        void UnloadModel(size_t bind)
        {
            if (m_VertexBuffers.empty() || bind > m_VertexBuffers.size() - 1)
                return;
            m_Device->WaitIdle();

            m_VertexBuffers.erase(m_VertexBuffers.begin() + bind);
            m_IndexBuffers.erase(m_IndexBuffers.begin() + bind);

            CreateCommandBuffers();
        }

        void DrawFrame() 
        {

            VkResult result = m_Swapchain->AcquireNextImage(imageAvailableSemaphores[currentFrame], inFlightFences[currentFrame]);

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

            currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
        }
};

VkExtent2D VulkanApp::ChooseSwapExtent(uint32_t width, uint32_t height)
{
    VkSurfaceCapabilitiesKHR capabilities {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(*m_PhysicalDevice, *m_Surface, &capabilities);

    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
    {
        return capabilities.currentExtent;
    } 
    else 
    {
        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width  = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

        return actualExtent;
    }
}


// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

int main() 
{
    Eternity::CreateWindow(800, 600, "Eternity");
    Eternity::EventSystem::Init();
    Eternity::Input::Init();
    Eternity::Input::SetMouseMode(Eternity::Input::MouseMode::Capture);

    Renderable model;

    auto& vertices = model.vertices;
    // front
    vertices.push_back({ .pos = glm::vec3(-0.5, -0.5, 0.5), .texCoord = glm::vec2(3.0f / 16.0f, 1.0f / 16.0f) });
    vertices.push_back({ .pos = glm::vec3(0.5, -0.5, 0.5), .texCoord = glm::vec2(4.0f / 16.0f, 1.0f / 16.0f) });
    vertices.push_back({ .pos = glm::vec3(-0.5, 0.5, 0.5), .texCoord = glm::vec2(3.0f / 16.0f, 0.0f) });
    vertices.push_back({ .pos = glm::vec3(0.5, 0.5, 0.5), .texCoord = glm::vec2(4.0f / 16.0f, 0.0f) });
    // right
    vertices.push_back({ .pos = glm::vec3(0.5, -0.5, 0.5), .texCoord = glm::vec2(0, 0) });
    vertices.push_back({ .pos = glm::vec3(0.5, -0.5, -0.5), .texCoord = glm::vec2(0, 0) });
    vertices.push_back({ .pos = glm::vec3(0.5, 0.5, 0.5), .texCoord = glm::vec2(0, 0) });
    vertices.push_back({ .pos = glm::vec3(0.5, 0.5, -0.5), .texCoord = glm::vec2(0, 0) });

    // back
    vertices.push_back({ .pos = glm::vec3(0.5, -0.5, -0.5), .texCoord = glm::vec2(0, 0) });
    vertices.push_back({ .pos = glm::vec3(-0.5, -0.5, -0.5), .texCoord = glm::vec2(0, 0) });
    vertices.push_back({ .pos = glm::vec3(0.5, 0.5, -0.5), .texCoord = glm::vec2(0, 0) });
    vertices.push_back({ .pos = glm::vec3(-0.5, 0.5, -0.5), .texCoord = glm::vec2(0, 0) });

    // left
    vertices.push_back({ .pos = glm::vec3(-0.5, -0.5, -0.5), .texCoord = glm::vec2(0, 0) });
    vertices.push_back({ .pos = glm::vec3(-0.5, -0.5, 0.5), .texCoord = glm::vec2(0, 0) });
    vertices.push_back({ .pos = glm::vec3(-0.5, 0.5, -0.5), .texCoord = glm::vec2(0, 0) });
    vertices.push_back({ .pos = glm::vec3(-0.5, 0.5, 0.5), .texCoord = glm::vec2(0, 0) });

    // up
    vertices.push_back({ .pos = glm::vec3(-0.5, 0.5, -0.5), .texCoord = glm::vec2(0, 0) });
    vertices.push_back({ .pos = glm::vec3(-0.5, 0.5, 0.5), .texCoord = glm::vec2(0, 1.0f / 16.0f) });
    vertices.push_back({ .pos = glm::vec3(0.5, 0.5, -0.5), .texCoord = glm::vec2(1.0f / 16.0f, 0) });
    vertices.push_back({ .pos = glm::vec3(0.5, 0.5, 0.5), .texCoord = glm::vec2(1.0f / 16.0f, 1.0f / 16.0f) });

    // down
    vertices.push_back({ .pos = glm::vec3(0.5, -0.5, -0.5), .texCoord = glm::vec2(0, 0) });
    vertices.push_back({ .pos = glm::vec3(0.5, -0.5, 0.5), .texCoord = glm::vec2(0, 0) });
    vertices.push_back({ .pos = glm::vec3(-0.5, -0.5, -0.5), .texCoord = glm::vec2(0, 0) });
    vertices.push_back({ .pos = glm::vec3(-0.5, -0.5, 0.5), .texCoord = glm::vec2(0, 0) });

    auto& indices = model.indices;
    indices = std::vector<uint32_t> { 0, 1, 2, 2, 1, 3, 4, 5, 6, 6, 5, 7, 8, 9, 10, 10, 9, 11, 12, 13, 14, 14, 13, 15, 16, 17, 18, 18, 17, 19, 20, 21, 22, 22, 21, 23 };

    VulkanApp app;
    
    size_t bind;

    std::shared_ptr<Camera> camera = std::make_shared<Camera>(glm::vec3(0.0f, 0.0f, 3.0f));
    app.SetRenderCamera(camera);

    while (!Eternity::WindowShouldClose()) 
    {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        camera->Update(deltaTime);

        if (Input::GetKeyDown(Key::C))
        {
            app.LoadModel(model, bind);
            for (auto& i : vertices)
                i.pos.x += 1.0f;
        }
        
        if (Input::GetKeyDown(Key::X))
        {
            for (auto& i : vertices)
                i.pos.x -= 1.0f;
            app.UnloadModel(bind);
            bind--;
        }

        EventSystem::PollEvents();
        app.DrawFrame();
    }


    Eternity::DestroyWindow();

    return EXIT_SUCCESS;
}