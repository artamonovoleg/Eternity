#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// #define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "VulkanApp.hpp"

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

#include "Renderable.hpp"
#include "Camera.hpp"

const int MAX_FRAMES_IN_FLIGHT = 2;
const std::string TEXTURE_PATH = "../textures/atlas.png";

namespace Eternity
{
    VulkanApp::VulkanApp()
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

    VulkanApp::~VulkanApp()
    {
        Cleanup();
    }

    void VulkanApp::SetRenderCamera(std::shared_ptr<Camera>& camera)
    {
        m_RenderCamera = camera;
    }

    void VulkanApp::LoadModel(Renderable& model, size_t& bind) 
    {
        m_Device->WaitIdle();
        
        m_VertexBuffers.emplace_back(CreateVertexBuffer(*m_CommandPool, model.vertices.data(), sizeof(model.vertices[0]) * model.vertices.size()));
        m_IndexBuffers.emplace_back(CreateIndexBuffer(*m_CommandPool, model.indices.data(), sizeof(model.indices[0]) * model.indices.size()));

        CreateCommandBuffers();

        bind = m_VertexBuffers.size() - 1;
    }

    void VulkanApp::Prepare()
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

        m_TextureImage = std::make_shared<Image2D>(*m_CommandPool, TEXTURE_PATH, VK_FILTER_NEAREST);

        CreateSyncObjects();

        CreateGraphicsPipeline();
        
        CreateUniformBuffers();
        
        CreateDescriptorPool();
        CreateDescriptorSets();
        CreateCommandBuffers();
    }

    void VulkanApp::Cleanup()
    {
        m_Device->WaitIdle();
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroySemaphore(*m_Device, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(*m_Device, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(*m_Device, inFlightFences[i], nullptr);
        }
    }

    void VulkanApp::RecreateSwapchain(const VkExtent2D& extent)
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

    void VulkanApp::CreateRenderPass() 
    {
        Attachment colorAttachment(Attachment::Type::Color, 0, m_Swapchain->GetImageFormat());
        Attachment depthAttachment(Attachment::Type::Depth, 1, m_DepthImage->GetFormat());

        m_RenderPass = std::make_shared<RenderPass>(*m_Device, std::vector{ colorAttachment }, depthAttachment);
    }

    void VulkanApp::CreateDescriptorSetLayout() 
    {
        VkDescriptorSetLayoutBinding uboLayoutBinding       = UniformBuffer::GetDescriptorSetLayout(0, 1);
        VkDescriptorSetLayoutBinding samplerLayoutBinding   = Image2D::GetDescriptorSetLayout(1, 1);

        std::vector<VkDescriptorSetLayoutBinding> bindings = { uboLayoutBinding, samplerLayoutBinding };
        m_DescriptorSetLayout = std::make_shared<DescriptorSetLayout>(*m_Device, bindings);
    }

    void VulkanApp::CreateGraphicsPipeline() 
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

    void VulkanApp::CreateUniformBuffers() 
    {
        m_UniformBuffers.resize(m_Swapchain->GetImageCount());

        for (size_t i = 0; i < m_UniformBuffers.size(); i++) 
            m_UniformBuffers[i] = std::make_shared<UniformBuffer>(*m_Device, sizeof(UBOMatrices));
    }

    void VulkanApp::CreateDescriptorPool() 
    {
        const std::vector<DescriptorType> descriptorTypes { DescriptorType::Uniform, DescriptorType::ImageSampler };
        m_DescriptorPool = std::make_shared<DescriptorPool>(*m_Swapchain, descriptorTypes);
    }

    void VulkanApp::CreateDescriptorSets() 
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

    void VulkanApp::CreateCommandBuffers() 
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

    void VulkanApp::CreateSyncObjects() 
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

    void VulkanApp::UpdateUniformBuffer(uint32_t currentImage) 
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

    void VulkanApp::UnloadModel(size_t bind)
    {
        if (m_VertexBuffers.empty() || bind > m_VertexBuffers.size() - 1)
            return;
        m_Device->WaitIdle();

        m_VertexBuffers.erase(m_VertexBuffers.begin() + bind);
        m_IndexBuffers.erase(m_IndexBuffers.begin() + bind);

        CreateCommandBuffers();
    }

    void VulkanApp::DrawFrame() 
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
}