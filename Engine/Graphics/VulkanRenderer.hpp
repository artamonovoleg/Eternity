#pragma once

#include <vulkan/vulkan.h>
#include "Window.hpp"
#include "Mesh.hpp"
#include "Camera.hpp"
#include "DeletionQueue.hpp"

namespace Eternity
{
    class VulkanRenderer
    {
        private:
            VkInstance                  m_Instance          = VK_NULL_HANDLE;
            VkDebugUtilsMessengerEXT    m_DebugMessenger    = VK_NULL_HANDLE;
            VkPhysicalDevice            m_GPU               = VK_NULL_HANDLE;
            VkDevice                    m_Device            = VK_NULL_HANDLE;
            VkSurfaceKHR                m_Surface           = VK_NULL_HANDLE;

            VkSwapchainKHR              m_Swapchain             = VK_NULL_HANDLE;
            std::vector<VkImage>        m_SwapchainImages       = {};
            VkFormat                    m_SwapchainImageFormat  = {};
            VkExtent2D                  m_SwapchainExtent       = {};
            std::vector<VkImageView>    m_SwapchainImageViews   = {};

            VkQueue                     m_GraphicsQueue         = VK_NULL_HANDLE;
            uint32_t                    m_GraphicsQueueFamily   = 0; 
            VkQueue                     m_PresentQueue          = VK_NULL_HANDLE;
            uint32_t                    m_PresentQueueFamily    = 0;

            VkCommandPool               m_CommandPool           = VK_NULL_HANDLE;
            VkCommandBuffer             m_CommandBuffer         = VK_NULL_HANDLE;

            VkRenderPass                m_RenderPass            = VK_NULL_HANDLE;
            std::vector<VkFramebuffer>  m_Framebuffers          = {};
            
            DeletionQueue               m_DeletionQueue         = {};
            // render loop
            int                         m_FrameNumber           = 0;
            VkSemaphore                 m_PresentSemaphore      = VK_NULL_HANDLE;
            VkSemaphore                 m_RenderSemaphore       = VK_NULL_HANDLE;
            VkFence                     m_RenderFence           = VK_NULL_HANDLE;

            VkPipeline                  m_Pipeline              = VK_NULL_HANDLE;
            VkPipelineLayout            m_PipelineLayout        = VK_NULL_HANDLE;

            VkDeviceMemory              m_VertexBufferMemory    = VK_NULL_HANDLE;
            VkPipeline                  m_MeshPipeline          = VK_NULL_HANDLE;
            VkPipelineLayout            m_MeshPipelineLayout    = VK_NULL_HANDLE;
            Mesh                        m_Mesh                  = {};

            VkImageView                 m_DepthImageView        = VK_NULL_HANDLE;
            VkImage                     m_DepthImage            = VK_NULL_HANDLE;
            VkDeviceMemory              m_DepthImageMemory      = VK_NULL_HANDLE;
            VkFormat                    m_DepthFormat           = {};

            // camera
            Camera                      m_Camera                = glm::vec3(0.0f, 0.0f, 3.0f);
            void InitInstance();
            void CreateSurface();
            void CreateDevice();
            void CreateSwapchain();
            void InitCommands();
            void CreateRenderPass();
            void CreateFramebuffers();
            void CreateSyncObjects();
            void DestroySyncObjects();
            void CreatePipeline();

            void LoadMeshes();
            void UploadMesh(Mesh& mesh);
        public:
            void InitVulkan();
            void DeinitVulkan();
            void Draw();
    };
}