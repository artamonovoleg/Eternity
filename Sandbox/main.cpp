#include <vector>
#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "Assert.hpp"
#include "Log.hpp"
#include "VkDebugHelper.hpp"
#include "VkRendererAPI.hpp"

int main()
{
    glfwInit();
    int width = 800;
    int height = 600;
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(width, height, "Eternity", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    std::shared_ptr<Eternity::VkRendererAPI> r = std::make_shared<Eternity::VkRendererAPI>();

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    uint32_t surfaceWidth;
    uint32_t surfaceHeight;
    VkResult result = glfwCreateWindowSurface(r->GetVulkanInstance(), window, nullptr, &surface);
    VkDebugHelper::CheckResult(result);

    // init OsSpecificSurface
    {
        VkBool32 WSI_SUPPORTED = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(r->GetVulkanPhysicalDevice(), r->GetVulkanGraphicsFamilyIndex(), surface, &WSI_SUPPORTED);
        ET_CORE_ASSERT(WSI_SUPPORTED, "WSI is not supported");
    }
    // init surface
    VkSurfaceFormatKHR surfaceFormat{};
    VkSurfaceCapabilitiesKHR surfaceCapabilities{};
    {
        auto gpu = r->GetVulkanPhysicalDevice();
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surfaceCapabilities);

        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, nullptr);

        if (surfaceCapabilities.currentExtent.width < UINT32_MAX)
        {
            surfaceWidth    = surfaceCapabilities.currentExtent.width;
            surfaceHeight   = surfaceCapabilities.currentExtent.height;
        }

        ET_CORE_ASSERT(formatCount > 0, "Surface formats missing");
        std::vector<VkSurfaceFormatKHR> formats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, formats.data());
        if (formats[0].format == VK_FORMAT_UNDEFINED)
        {
            surfaceFormat.format        = VK_FORMAT_B8G8R8A8_UNORM;
            surfaceFormat.colorSpace    = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
        }
        else
        {
            surfaceFormat               = formats[0];
        }
    }

    VkSwapchainKHR swapchain;
    uint32_t swapchainImageCount = 2;
    // Init swapchain
    {

        if (swapchainImageCount > surfaceCapabilities.maxImageCount ) swapchainImageCount = surfaceCapabilities.maxImageCount;
        if (swapchainImageCount < surfaceCapabilities.minImageCount ) swapchainImageCount = surfaceCapabilities.minImageCount + 1;

        VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
        {
            uint32_t presentModeCount = 0;
            vkGetPhysicalDeviceSurfacePresentModesKHR(r->GetVulkanPhysicalDevice(), surface, &presentModeCount, nullptr);
            std::vector<VkPresentModeKHR> presentModeList(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(r->GetVulkanPhysicalDevice(), surface, &presentModeCount, presentModeList.data());
            for (auto m : presentModeList)
            {
                if (m == VK_PRESENT_MODE_MAILBOX_KHR)
                    presentMode = m;
            }
        }

        VkSwapchainCreateInfoKHR swapchainCreateInfo{};
        swapchainCreateInfo.sType                   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainCreateInfo.surface                 = surface;
        swapchainCreateInfo.minImageCount           = swapchainImageCount;
        swapchainCreateInfo.imageFormat             = surfaceFormat.format;
        swapchainCreateInfo.imageColorSpace         = surfaceFormat.colorSpace;
        swapchainCreateInfo.imageExtent.width       = surfaceWidth;
        swapchainCreateInfo.imageExtent.height      = surfaceHeight;
        swapchainCreateInfo.imageArrayLayers        = 1;
        swapchainCreateInfo.imageUsage              = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapchainCreateInfo.imageSharingMode        = VK_SHARING_MODE_EXCLUSIVE;
        swapchainCreateInfo.queueFamilyIndexCount   = 0;
        swapchainCreateInfo.pQueueFamilyIndices     = VK_NULL_HANDLE;
        swapchainCreateInfo.preTransform            = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        swapchainCreateInfo.compositeAlpha          = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapchainCreateInfo.presentMode             = presentMode;
        swapchainCreateInfo.clipped                 = VK_TRUE;
        swapchainCreateInfo.oldSwapchain            = VK_NULL_HANDLE;

        VkDebugHelper::CheckResult(vkCreateSwapchainKHR(r->GetVulkanDevice(), &swapchainCreateInfo, nullptr, &swapchain));
        VkDebugHelper::CheckResult(vkGetSwapchainImagesKHR(r->GetVulkanDevice(), swapchain, &swapchainImageCount, nullptr));
    }

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }

    // deinit swapchain
    {
        vkDestroySwapchainKHR(r->GetVulkanDevice(), swapchain, nullptr);
    }

    vkDestroySurfaceKHR(r->GetVulkanInstance(), surface, nullptr);
    glfwTerminate();
    return VK_SUCCESS;
}