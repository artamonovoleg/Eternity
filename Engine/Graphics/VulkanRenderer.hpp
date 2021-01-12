#pragma once

#include <vulkan/vulkan.h>
#include "Window.hpp"
#include "Mesh.hpp"
#include "Camera.hpp"
#include "DeletionQueue.hpp"

namespace Eternity
{
    struct FrameData 
    {
        VkSemaphore presentSemaphore;
        VkSemaphore renderSemaphore;
        VkFence     renderFence;	

        VkCommandPool   commandPool;
        VkCommandBuffer commandBuffer;

        VkBuffer 		cameraBuffer;
	    VkDescriptorSet globalDescriptor;
    };

    static constexpr unsigned int FRAME_OVERLAP = 2;

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
            FrameData                   m_Frames[FRAME_OVERLAP] = {};
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

            //default array of renderable objects
            std::vector<RenderObject>                       m_Renderables;

            std::unordered_map<std::string, Material>       m_Materials;
            std::unordered_map<std::string, Mesh>           m_Meshes;

            FrameData&                  GetCurrentFrame();

            //create material and add it to the map
            std::shared_ptr<Material>   CreateMaterial(VkPipeline pipeline, VkPipelineLayout layout, const std::string& name);

            //returns nullptr if it can't be found
            std::shared_ptr<Material>   GetMaterial(const std::string& name);

            //returns nullptr if it can't be found
            std::shared_ptr<Mesh>       GetMesh(const std::string& name);

            void                        DrawObjects(VkCommandBuffer cmd, std::vector<RenderObject>& first, int count);

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
            void InitScene();
        public:
            void InitVulkan();
            void DeinitVulkan();
            void Draw();
    };
}