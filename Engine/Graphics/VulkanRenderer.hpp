#pragma once

#include <cstring>
#include <vulkan/vulkan.h>
#include "Window.hpp"
#include "Camera.hpp"
#include "DeletionQueue.hpp"

namespace Eternity
{
    class VulkanRenderer
    {
        private:
            // memory managment
            DeletionQueue   m_DeletionQueue     = {};
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

            VkRenderPass                m_RenderPass        = VK_NULL_HANDLE;
            VkPipelineLayout            m_PipelineLayout    = VK_NULL_HANDLE;
            VkPipeline                  m_GraphicsPipeline  = VK_NULL_HANDLE;

            std::vector<VkFramebuffer>      m_SwapchainFramebuffers     = {};

            VkCommandPool                   m_CommandPool               = VK_NULL_HANDLE;
            std::vector<VkCommandBuffer>    m_CommandBuffers            = {};

            std::vector<VkSemaphore>        m_ImageAvailableSemaphores  = {};
            std::vector<VkSemaphore>        m_RenderFinishedSemaphores  = {};
            std::vector<VkFence>            m_InFlightFences            = {};
            std::vector<VkFence>            m_ImagesInFlight            = {};
            size_t                          m_CurrentFrame              = 0;

            bool                            m_FramebufferResized        = false;

            VkBuffer                        m_VertexBuffer              = VK_NULL_HANDLE;
            VkDeviceMemory                  m_VertexBufferMemory        = VK_NULL_HANDLE;
            
            // Init vulkan
            void InitInstance();
            void CreateSurface();
            void CreateLogicalDevice();
            void CreateSwapchain();

            void CreateRenderPass();
            // create pipeline
            void CreateGraphicsPipeline();
            void CreateFramebuffers();

            void CreateCommandPool();
            void CreateVertexBuffer();
            void CreateCommandBuffers();

            void CreateSyncObjects();

            void RecreateSwapchain();

        public:
            void InitVulkan();
            void DeinitVulkan();

            void DrawFrame();
    };
}