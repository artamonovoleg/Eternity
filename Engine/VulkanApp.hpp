#pragma once

#include <memory>
#include <vector>
#include <cstring>
#include <vulkan/vulkan.h>

namespace Eternity
{
    class Instance;
    class Surface;
    class PhysicalDevice;
    class Device;
    class Swapchain;
    class DepthImage;
    class RenderPass;
    class Framebuffers;
    class CommandPool;
    class Image2D;
    class DescriptorSetLayout;
    class GraphicsPipelineLayout;
    class GraphicsPipeline;
    class Buffer;
    class UniformBuffer;
    class DescriptorPool;
    class DescriptorSets;
    class CommandBuffer;
    class Camera;
    class Renderable;
    
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

            void Prepare();
            void Cleanup();

            void RecreateSwapchain(const VkExtent2D& extent);
            void CreateRenderPass();
            void CreateDescriptorSetLayout();
            void CreateGraphicsPipeline();
            void CreateUniformBuffers();
            void CreateDescriptorPool();
            void CreateDescriptorSets();
            void CreateCommandBuffers();
            void CreateSyncObjects();
            void UpdateUniformBuffer(uint32_t currentImage);
        public:
            VulkanApp();
            ~VulkanApp();
            
            void SetRenderCamera(std::shared_ptr<Camera>& camera);
            void LoadModel(Renderable& model);
            void UnloadModel(Renderable& model);
            void DrawFrame();
    };
}