#include <GLFW/glfw3.h>
#include "Swapchain.hpp"
#include "VkCheck.hpp"
#include "Surface.hpp"
#include "PhysicalDevice.hpp"
#include "Device.hpp"

namespace Eternity
{
    Swapchain::Swapchain(const VkExtent2D& extent, const Device& device)
        : m_Extent(extent), m_Device(device)
    {
        CreateSwapchain();
        CreateImageViews();
    }
    
    Swapchain::~Swapchain()
    {
        CleanupSwapchain();
    }

    void Swapchain::Recreate(const VkExtent2D& currentExtent)
    {
        m_Extent = currentExtent;

        CleanupSwapchain();
        CreateSwapchain();
        CreateImageViews();
    }

    VkResult Swapchain::AcquireNextImage(const VkSemaphore &presentCompleteSemaphore, VkFence fence)
    {
        if (fence != VK_NULL_HANDLE)
            vkWaitForFences(m_Device, 1, &fence, VK_TRUE, std::numeric_limits<uint64_t>::max());

        VkResult aquireResult = vkAcquireNextImageKHR(m_Device, m_Swapchain, std::numeric_limits<uint64_t>::max(), presentCompleteSemaphore, VK_NULL_HANDLE, &m_ActiveImageIndex);

        return aquireResult;
    }

    VkResult Swapchain::QueuePresent(const VkQueue &presentQueue, const VkSemaphore &waitSemaphore)
    {
        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType               = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount  = 1;
        presentInfo.pWaitSemaphores     = &waitSemaphore;
        presentInfo.swapchainCount      = 1;
        presentInfo.pSwapchains         = &m_Swapchain;
        presentInfo.pImageIndices       = &m_ActiveImageIndex;
        return vkQueuePresentKHR(presentQueue, &presentInfo);
    }

    void Swapchain::CreateSwapchain()
    {
        const PhysicalDevice&   physicalDevice      = m_Device.GetPhysicalDevice();
        VkSurfaceKHR            surface             = physicalDevice.GetSurface();
        auto                    swapchainSupport    = physicalDevice.GetSwapchainSupportDetails();

        VkSurfaceFormatKHR      surfaceFormat       = ChooseSurfaceFormat(swapchainSupport.formats);
        VkPresentModeKHR        presentMode         = ChoosePresentMode(swapchainSupport.presentModes);

        uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
        if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount)
            imageCount = swapchainSupport.capabilities.maxImageCount;

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface  = surface;

        createInfo.minImageCount    = imageCount;
        createInfo.imageFormat      = surfaceFormat.format;
        createInfo.imageColorSpace  = surfaceFormat.colorSpace;
        createInfo.imageExtent      = m_Extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        uint32_t m_GraphicsFamilyIndex  = physicalDevice.GetQueueFamilyIndex(QueueType::Graphics);
        uint32_t m_PresentFamilyIndex   = physicalDevice.GetQueueFamilyIndex(QueueType::Present);
        uint32_t queueFamilyIndices[] = { m_GraphicsFamilyIndex, m_PresentFamilyIndex };

        if (m_GraphicsFamilyIndex != m_PresentFamilyIndex) 
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } 
        else 
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform     = swapchainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode      = presentMode;
        createInfo.clipped          = VK_TRUE;

        VkCheck(vkCreateSwapchainKHR(m_Device, &createInfo, nullptr, &m_Swapchain));

        vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &imageCount, nullptr);
        m_Images.resize(imageCount);
        vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &imageCount, m_Images.data());

        m_ImageCount    = imageCount; 
        m_ImageFormat   = surfaceFormat.format;

        ET_TRACE("Swapchain created");
    }

    void Swapchain::CreateImageViews() 
    {
        m_ImageViews.resize(m_Images.size());

        for (uint32_t i = 0; i < m_Images.size(); i++)
            m_ImageViews[i] = m_Device.CreateImageView(m_Images[i], m_ImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
        ET_TRACE("Swapchain image views created");
    }

    void Swapchain::CleanupSwapchain()
    {
        for (auto imageView : m_ImageViews) 
            vkDestroyImageView(m_Device, imageView, nullptr);
        ET_TRACE("Swapchain image views destroyed");

        vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);
        ET_TRACE("Swapchain destroyed");
    }

    VkSurfaceFormatKHR Swapchain::ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const
    {
        for (const auto& availableFormat : availableFormats) 
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
                return availableFormat;
        }

        return availableFormats[0];
    }

    VkPresentModeKHR Swapchain::ChoosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const
    {
        for (const auto& availablePresentMode : availablePresentModes) 
        {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
                return availablePresentMode;
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    const uint32_t                      Swapchain::GetImageCount()         const { return m_ImageCount; }
    const VkFormat                      Swapchain::GetImageFormat()        const { return m_ImageFormat; }
    const VkExtent2D                    Swapchain::GetExtent()             const { return m_Extent; }
    const std::vector<VkImageView>&     Swapchain::GetImageViews()         const { return m_ImageViews; };
    const std::vector<VkFramebuffer>&   Swapchain::GetFramebuffers()       const { return m_Framebuffers; }
    const uint32_t&                     Swapchain::GetActiveImageIndex()   const { return m_ActiveImageIndex; }
    const Device&                       Swapchain::GetDevice()             const { return m_Device; }
} // namespace Eternity
