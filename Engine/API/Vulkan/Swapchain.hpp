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
            std::vector<VkImage>        m_SwapchainImages;
            VkFormat                    m_SwapchainImageFormat;
            VkExtent2D                  m_SwapchainExtent;
            std::vector<VkImageView>    m_SwapchainImageViews;
            std::vector<VkFramebuffer>  m_SwapchainFramebuffers;

            VkSurfaceFormatKHR  ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const;
            VkPresentModeKHR    ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const;
            VkExtent2D          ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;
            
            void CreateSwapchain();
            void CreateImageViews();
        public:
            Swapchain(const Device& device);
            ~Swapchain();

            const std::vector<VkImage>& GetImages() const { return m_SwapchainImages; }
            const VkFormat GetImageFormat() const { return m_SwapchainImageFormat; }
            const VkExtent2D GetExtent() const { return m_SwapchainExtent; }
            const std::vector<VkImageView>& GetImageViews() const { return m_SwapchainImageViews; };
            const std::vector<VkFramebuffer>& GetFramebuffers() const { return m_SwapchainFramebuffers; }

            operator VkSwapchainKHR() { return m_Swapchain; }
    };    
} // namespace Eternity
