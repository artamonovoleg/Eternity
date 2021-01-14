#pragma once

#include <cstring>
#include <unordered_map>
#include <vulkan/vulkan.h>
#include <stb_image.h>
#include "Window.hpp"
#include "Camera.hpp"
#include "DeletionQueue.hpp"
#include "Vertex.hpp"

namespace Eternity
{
    struct UniformBufferObject 
    {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };

    class VulkanRenderer
    {
        private:
            // memory managment
            DeletionQueue               m_DeletionQueue     = {};
            // Vulkan structures
            VkInstance                  m_Instance          = VK_NULL_HANDLE;
            VkDebugUtilsMessengerEXT    m_DebugMessenger    = VK_NULL_HANDLE;

            VkSurfaceKHR                m_Surface           = VK_NULL_HANDLE;

            VkPhysicalDevice            m_GPU               = VK_NULL_HANDLE;
            VkDevice                    m_Device            = VK_NULL_HANDLE;

            uint32_t                    m_GraphicsFamily    = 0;
            uint32_t                    m_PresentFamily     = 0;
            VkQueue                     m_GraphicsQueue     = VK_NULL_HANDLE;
            VkQueue                     m_PresentQueue      = VK_NULL_HANDLE;

            VkSwapchainKHR              m_Swapchain         = VK_NULL_HANDLE;
            std::vector<VkImage>        m_Images            = {};
            VkFormat                    m_ImageFormat       = {};
            VkExtent2D                  m_Extent            = {};
            std::vector<VkImageView>    m_ImageViews        = {};

            VkRenderPass                m_RenderPass                    = VK_NULL_HANDLE;
            VkDescriptorSetLayout       m_DescriptorSetLayout           = VK_NULL_HANDLE;
            VkPipelineLayout            m_PipelineLayout                = VK_NULL_HANDLE;
            VkPipeline                  m_GraphicsPipeline              = VK_NULL_HANDLE;

            std::vector<VkFramebuffer>      m_SwapchainFramebuffers     = {};

            VkCommandPool                   m_CommandPool               = VK_NULL_HANDLE;
            std::vector<VkCommandBuffer>    m_CommandBuffers            = {};

            std::vector<VkSemaphore>        m_ImageAvailableSemaphores  = {};
            std::vector<VkSemaphore>        m_RenderFinishedSemaphores  = {};
            std::vector<VkFence>            m_InFlightFences            = {};
            std::vector<VkFence>            m_ImagesInFlight            = {};
            size_t                          m_CurrentFrame              = 0;

            bool                            m_FramebufferResized        = false;
            
            uint32_t                        m_MipLevels                 = 0;
            VkImage                         m_TextureImage              = VK_NULL_HANDLE;
            VkImageView                     m_TextureImageView          = VK_NULL_HANDLE;
            VkSampler                       m_TextureSampler            = VK_NULL_HANDLE;
            VkDeviceMemory                  m_TextureImageMemory        = VK_NULL_HANDLE;

            VkImage                         m_DepthImage                = VK_NULL_HANDLE;
            VkDeviceMemory                  m_DepthImageMemory          = VK_NULL_HANDLE;
            VkImageView                     m_DepthImageView            = VK_NULL_HANDLE;

            std::vector<Vertex>             m_Vertices                  = {};
            std::vector<uint32_t>           m_Indices                   = {};
            VkBuffer                        m_VertexBuffer              = VK_NULL_HANDLE;
            VkDeviceMemory                  m_VertexBufferMemory        = VK_NULL_HANDLE;
            VkBuffer                        m_IndexBuffer               = VK_NULL_HANDLE;
            VkDeviceMemory                  m_IndexBufferMemory         = VK_NULL_HANDLE;
            std::vector<VkBuffer>           m_UniformBuffers            = {};
            std::vector<VkDeviceMemory>     m_UniformBuffersMemory      = {};
            VkDescriptorPool                m_DescriptorPool            = VK_NULL_HANDLE;
            std::vector<VkDescriptorSet>    m_DescriptorSets            = {};
            
            // obj loader
            Camera                          m_Camera                    = Camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            std::string                     m_TexturePath               = {};
            std::string                     m_ModelPath                 = {};

            // Init vulkan
            void InitInstance();
            void CreateSurface();
            void CreateLogicalDevice();
            void CreateSwapchain();
            void CleanupSwapchain();

            void CreateRenderPass();
            void CreateDescriptorSetLayout();
            // create pipeline
            void CreateGraphicsPipeline();
            void CreateFramebuffers();

            void CreateCommandPool();
            void CreateTextureImage();
            VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
            void CreateTextureImageView();
            void CreateTextureSampler();
            void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
            void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
            void GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
            bool HasStencilComponent(VkFormat format);
            void CreateDepthResources();    

            void LoadModel();
            void CreateVertexBuffer();
            void CreateIndexBuffer();
            void CreateUniformBuffers();
            void UpdateUniformBuffer(uint32_t currentImage);
            void CreateDescriptorPool();
            void CreateDescriptorSets();
            void CreateCommandBuffers();

            void CreateSyncObjects();

            void RecreateSwapchain();
        public:
            void InitVulkan();
            void SetTargetModel(const std::string& modelPath, const std::string& texturePath);
            void DeinitVulkan();

            void DrawFrame();
    };
}