#include "VulkanHelper.hpp"
#include "Window.hpp"

namespace vkh
{
    VkSurfaceFormatKHR                              ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
        for (const auto& availableFormat : availableFormats) 
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return availableFormat;
        }

        return availableFormats[0];
    }

    VkPresentModeKHR                                ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) 
    {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D                                      ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) 
    {
        if (capabilities.currentExtent.width != UINT32_MAX) 
        {
            return capabilities.currentExtent;
        } 
        else 
        {
            int width, height;
            glfwGetFramebufferSize(Eternity::GetCurrentWindow(), &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

            return actualExtent;
        }
    }

    SwapchainSupportDetails                         QuerySwapchainSupport(const VkPhysicalDevice& device, const VkSurfaceKHR& surface)
    {
        SwapchainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

        if (formatCount != 0) 
        {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

        if (presentModeCount != 0) 
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    Swapchain                                       BuildSwapchain(const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& surface, const VkDevice& device)
    {
        Swapchain       swapchain   = {};

        SwapchainSupportDetails swapchainSupport    = QuerySwapchainSupport(physicalDevice, surface);

        VkSurfaceFormatKHR surfaceFormat            = ChooseSwapSurfaceFormat(swapchainSupport.formats);
        VkPresentModeKHR presentMode                = ChooseSwapPresentMode(swapchainSupport.presentModes);
        VkExtent2D extent                           = ChooseSwapExtent(swapchainSupport.capabilities);

        uint32_t imageCount = swapchainSupport.capabilities.minImageCount + 1;
        if (swapchainSupport.capabilities.maxImageCount > 0 && imageCount > swapchainSupport.capabilities.maxImageCount) 
            imageCount = swapchainSupport.capabilities.maxImageCount;

        VkSwapchainCreateInfoKHR swapchainCI
        {
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface = surface,

            .minImageCount = imageCount,
            .imageFormat = surfaceFormat.format,
            .imageColorSpace = surfaceFormat.colorSpace,
            .imageExtent = extent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
        };

        vkh::QueueFamilyIndices indices = vkh::FindQueueFamilies(physicalDevice, surface);
        uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        if (indices.graphicsFamily != indices.presentFamily)
        {
            swapchainCI.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapchainCI.queueFamilyIndexCount = 2;
            swapchainCI.pQueueFamilyIndices = queueFamilyIndices;
        } 
        else 
        {
            swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        swapchainCI.preTransform = swapchainSupport.capabilities.currentTransform;
        swapchainCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchainCI.presentMode = presentMode;
        swapchainCI.clipped = VK_TRUE;

        swapchainCI.oldSwapchain = VK_NULL_HANDLE;

        auto res = vkCreateSwapchainKHR(device, &swapchainCI, nullptr, &swapchain.swapchain);
        vkh::Check(res, "Swapchain create failed");

        vkGetSwapchainImagesKHR(device, swapchain.swapchain, &imageCount, nullptr);
        swapchain.images.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapchain.swapchain, &imageCount, swapchain.images.data());

        swapchain.format    = surfaceFormat.format;
        swapchain.extent    = extent;

        return swapchain;
    }
}