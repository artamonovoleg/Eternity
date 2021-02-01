#pragma once
#include <vector>
#include <vulkan/vulkan.h>

namespace Eternity
{
    class PhysicalDevice;
    class Device;

    class Swapchain
    {
        private:
            const Device&               m_Device;

            VkSwapchainKHR              m_Swapchain;
            std::vector<VkImage>        m_Images;
            uint32_t                    m_ImageCount;
            VkFormat                    m_ImageFormat;
            VkExtent2D                  m_Extent;
            std::vector<VkImageView>    m_ImageViews;
            std::vector<VkFramebuffer>  m_Framebuffers;

            uint32_t                    m_ActiveImageIndex;

            VkSurfaceFormatKHR  ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const;
            VkPresentModeKHR    ChoosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const;
            
            void CreateSwapchain();
            void CreateImageViews();
            void CleanupSwapchain();
        public:
            Swapchain(const VkExtent2D& extent, const Device& device);
            ~Swapchain();

            void Recreate(const VkExtent2D& currentExtent);

            VkResult AcquireNextImage(const VkSemaphore &presentCompleteSemaphore, VkFence fence);
            VkResult QueuePresent(const VkQueue &presentQueue, const VkSemaphore &waitSemaphore);

            const uint32_t                      GetImageCount()         const;    
            const VkFormat                      GetImageFormat()        const;     
            const VkExtent2D                    GetExtent()             const;       
            const std::vector<VkImageView>&     GetImageViews()         const;
            const std::vector<VkFramebuffer>&   GetFramebuffers()       const;    
            const uint32_t&                     GetActiveImageIndex()   const;  
            const Device&                       GetDevice()             const;
            
            operator VkSwapchainKHR() { return m_Swapchain; }
    };    
} // namespace Eternity
