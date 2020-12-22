#include <vulkan/vulkan.h>
#include <vector>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cassert>
#include "Assert.hpp"
#include "Log.hpp"
#include "VulkanDebug.hpp"
#include "VulkanRenderer.hpp"

namespace Eternity
{
    class VulkanGraphicContext
    {
        private:
            VkSurfaceKHR m_Surface      = VK_NULL_HANDLE;
            uint32_t m_SurfaceWidth     = 0;
            uint32_t m_SurfaceHeight    = 0;

            VkSurfaceFormatKHR m_SurfaceFormat{};
            VkSurfaceCapabilitiesKHR m_SurfaceCapabilities{};

            VkSwapchainKHR m_Swapchain{};
            uint32_t m_SwapchainImageCount = 2;

            std::vector<VkImage> m_SwapchainImages;
            std::vector<VkImageView> m_SwapchainImageViews;
            std::vector<VkFramebuffer> m_Framebuffers;

            VkImage m_DepthStencilImage                 = VK_NULL_HANDLE;
            VkImageView m_DepthStencilImageView         = VK_NULL_HANDLE;
            VkFormat    m_DepthStencilFormat            = VK_FORMAT_UNDEFINED;
            VkDeviceMemory m_DepthStencilImageMemory    = VK_NULL_HANDLE;
            bool m_StencilAvailable     = false;

            VkRenderPass m_RenderPass{};

            VulkanRenderer& m_Renderer;
            void SetupOSSurface();
            void InitSurface();
            void DeinitSurface();
            void InitSwapchain();
            void DeinitSwapchain();
            void InitSwapchainImages();
            void DeinitSwapchainImages();
            void InitDepthStencilImage();
            void DeinitDepthStencilImage();
            void InitRenderPass();
            void DeinitRenderPass();
            void InitFramebuffers();
            void DeinitFramebuffers();
        public:
            VulkanGraphicContext(VulkanRenderer &renderer, GLFWwindow* window);
            ~VulkanGraphicContext();
    };

    // create window surface
    VulkanGraphicContext::VulkanGraphicContext(VulkanRenderer &renderer, GLFWwindow* window)
        : m_Renderer(renderer)
    {
        VkResult result = glfwCreateWindowSurface(renderer.GetVulkanInstance(), window, nullptr, &m_Surface);
        VulkanDebug::CheckResult(result);
        SetupOSSurface();
        InitSurface();
        InitSwapchain();
        InitSwapchainImages();
        InitDepthStencilImage();
        InitRenderPass();
        InitFramebuffers();
    }

    VulkanGraphicContext::~VulkanGraphicContext()
    {
        DeinitFramebuffers();
        DeinitRenderPass();
        DeinitDepthStencilImage();
        DeinitSwapchainImages();
        DeinitSwapchain();
        DeinitSurface();
    }

    void VulkanGraphicContext::SetupOSSurface()
    {
        VkBool32 WSI_SUPPORTED = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(m_Renderer.GetVulkanPhysicalDevice(), m_Renderer.GetVulkanGraphicsFamilyIndex(), m_Surface, &WSI_SUPPORTED);
        ET_CORE_ASSERT(WSI_SUPPORTED, "WSI is not supported");
    }

    void VulkanGraphicContext::InitSurface()
    {
        auto gpu = m_Renderer.GetVulkanPhysicalDevice();
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, m_Surface, &m_SurfaceCapabilities);

        uint32_t formatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, m_Surface, &formatCount, nullptr);

        if (m_SurfaceCapabilities.currentExtent.width < UINT32_MAX)
        {
            m_SurfaceWidth    = m_SurfaceCapabilities.currentExtent.width;
            m_SurfaceHeight   = m_SurfaceCapabilities.currentExtent.height;
        }

        ET_CORE_ASSERT(formatCount > 0, "Surface formats missing");
        std::vector<VkSurfaceFormatKHR> formats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, m_Surface, &formatCount, formats.data());
        if (formats[0].format == VK_FORMAT_UNDEFINED)
        {
            m_SurfaceFormat.format        = VK_FORMAT_B8G8R8A8_UNORM;
            m_SurfaceFormat.colorSpace    = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
        }
        else
        {
            m_SurfaceFormat               = formats[0];
        }
    }

    void VulkanGraphicContext::DeinitSurface()
    {
        vkDestroySurfaceKHR(m_Renderer.GetVulkanInstance(), m_Surface, nullptr);
    }

    void VulkanGraphicContext::InitSwapchain()
    {
        if (m_SwapchainImageCount > m_SurfaceCapabilities.maxImageCount ) m_SwapchainImageCount = m_SurfaceCapabilities.maxImageCount;
        if (m_SwapchainImageCount < m_SurfaceCapabilities.minImageCount ) m_SwapchainImageCount = m_SurfaceCapabilities.minImageCount + 1;

        VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
        {
            uint32_t presentModeCount = 0;
            vkGetPhysicalDeviceSurfacePresentModesKHR(m_Renderer.GetVulkanPhysicalDevice(), m_Surface, &presentModeCount, nullptr);
            std::vector<VkPresentModeKHR> presentModeList(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(m_Renderer.GetVulkanPhysicalDevice(), m_Surface, &presentModeCount, presentModeList.data());
            for (auto m : presentModeList)
            {
                if (m == VK_PRESENT_MODE_MAILBOX_KHR)
                    presentMode = m;
            }
        }

        VkSwapchainCreateInfoKHR swapchainCreateInfo{};
        swapchainCreateInfo.sType                   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapchainCreateInfo.surface                 = m_Surface;
        swapchainCreateInfo.minImageCount           = m_SwapchainImageCount;
        swapchainCreateInfo.imageFormat             = m_SurfaceFormat.format;
        swapchainCreateInfo.imageColorSpace         = m_SurfaceFormat.colorSpace;
        swapchainCreateInfo.imageExtent.width       = m_SurfaceWidth;
        swapchainCreateInfo.imageExtent.height      = m_SurfaceHeight;
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

        VulkanDebug::CheckResult(vkCreateSwapchainKHR(m_Renderer.GetVulkanDevice(), &swapchainCreateInfo, nullptr, &m_Swapchain));
        VulkanDebug::CheckResult(vkGetSwapchainImagesKHR(m_Renderer.GetVulkanDevice(), m_Swapchain, &m_SwapchainImageCount, nullptr));
    }

    void VulkanGraphicContext::DeinitSwapchain()
    {
        vkDestroySwapchainKHR(m_Renderer.GetVulkanDevice(), m_Swapchain, nullptr);
    }

    void VulkanGraphicContext::InitSwapchainImages()
    {
        m_SwapchainImages.resize(m_SwapchainImageCount);
        m_SwapchainImageViews.resize(m_SwapchainImageCount);
        VulkanDebug::CheckResult(vkGetSwapchainImagesKHR(m_Renderer.GetVulkanDevice(), m_Swapchain, &m_SwapchainImageCount, m_SwapchainImages.data()), "Swapchain images");

        for (uint32_t i = 0; i < m_SwapchainImageCount; i++)
        {
            VkImageViewCreateInfo imageViewCreateInfo{};
            imageViewCreateInfo.sType           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewCreateInfo.image           = m_SwapchainImages[i];
            imageViewCreateInfo.viewType        = VK_IMAGE_VIEW_TYPE_2D;
            imageViewCreateInfo.format          = m_SurfaceFormat.format;
            imageViewCreateInfo.components.r    = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.g    = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.b    = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.a    = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            imageViewCreateInfo.subresourceRange.baseMipLevel   = 0;
            imageViewCreateInfo.subresourceRange.levelCount     = 1;
            imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
            imageViewCreateInfo.subresourceRange.layerCount     = 1;

            VulkanDebug::CheckResult(vkCreateImageView(m_Renderer.GetVulkanDevice(), &imageViewCreateInfo, nullptr, &m_SwapchainImageViews[i]));
        }
    }

    void VulkanGraphicContext::DeinitSwapchainImages()
    {
        for (auto& view : m_SwapchainImageViews)
            vkDestroyImageView(m_Renderer.GetVulkanDevice(), view, nullptr);
    }

    void VulkanGraphicContext::InitDepthStencilImage()
    {
        {
            std::vector<VkFormat> tryFormats {
                    VK_FORMAT_D32_SFLOAT_S8_UINT,
                    VK_FORMAT_D24_UNORM_S8_UINT,
                    VK_FORMAT_D16_UNORM_S8_UINT,
                    VK_FORMAT_D32_SFLOAT,
                    VK_FORMAT_D16_UNORM
            };

            for (auto& f : tryFormats)
            {
                VkFormatProperties formatProperties{};
                vkGetPhysicalDeviceFormatProperties(m_Renderer.GetVulkanPhysicalDevice(), f, &formatProperties);
                if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
                {
                    m_DepthStencilFormat = f;
                    break;
                }
            }

            ET_CORE_ASSERT(m_DepthStencilFormat != VK_FORMAT_UNDEFINED, "Depth stencil format not selected");

            if ((m_DepthStencilFormat == VK_FORMAT_D32_SFLOAT_S8_UINT) ||
                (m_DepthStencilFormat == VK_FORMAT_D24_UNORM_S8_UINT)  ||
                (m_DepthStencilFormat == VK_FORMAT_D16_UNORM_S8_UINT)  ||
                (m_DepthStencilFormat == VK_FORMAT_D32_SFLOAT)         ||
                (m_DepthStencilFormat == VK_FORMAT_D16_UNORM))
            {
                m_StencilAvailable = true;
            }
        }

        VkImageCreateInfo imageCreateInfo{};
        imageCreateInfo.sType                   = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.flags                   = 0;
        imageCreateInfo.imageType               = VK_IMAGE_TYPE_2D;
        imageCreateInfo.format                  = m_DepthStencilFormat;
        imageCreateInfo.extent.width            = m_SurfaceWidth;
        imageCreateInfo.extent.height           = m_SurfaceHeight;
        imageCreateInfo.extent.depth            = 1;
        imageCreateInfo.mipLevels               = 1;
        imageCreateInfo.arrayLayers             = 1;
        imageCreateInfo.samples                 = VK_SAMPLE_COUNT_1_BIT;
        imageCreateInfo.tiling                  = VK_IMAGE_TILING_OPTIMAL;
        imageCreateInfo.usage                   = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageCreateInfo.sharingMode             = VK_SHARING_MODE_EXCLUSIVE;
        imageCreateInfo.queueFamilyIndexCount   = VK_QUEUE_FAMILY_IGNORED;
        imageCreateInfo.pQueueFamilyIndices     = nullptr;
        imageCreateInfo.initialLayout           = VK_IMAGE_LAYOUT_UNDEFINED;

        VulkanDebug::CheckResult(vkCreateImage(m_Renderer.GetVulkanDevice(), &imageCreateInfo, nullptr, &m_DepthStencilImage), "Depth stencil image");

        VkMemoryRequirements imageMemoryRequirements{};
        vkGetImageMemoryRequirements(m_Renderer.GetVulkanDevice(), m_DepthStencilImage, &imageMemoryRequirements);

        uint32_t memoryIndex = UINT32_MAX;
        auto& gpuMemoryProperties = m_Renderer.GetVulkanPhysicalDeviceMemoryProperties();
        auto requiredProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

        for (uint32_t i = 0; i < gpuMemoryProperties.memoryTypeCount; i++)
        {
            if (imageMemoryRequirements.memoryTypeBits & ( 1 << i))
            {
                if (gpuMemoryProperties.memoryTypes[i].propertyFlags & requiredProperties)
                {
                    memoryIndex = i;
                    break;
                }
            }
        }

        ET_CORE_ASSERT(memoryIndex != UINT32_MAX, "Memory index");

        VkMemoryAllocateInfo memoryAllocateInfo{};
        memoryAllocateInfo.sType            = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memoryAllocateInfo.allocationSize   = imageMemoryRequirements.size;
        memoryAllocateInfo.memoryTypeIndex = memoryIndex;

        vkAllocateMemory(m_Renderer.GetVulkanDevice(), &memoryAllocateInfo, nullptr, &m_DepthStencilImageMemory);
        vkBindImageMemory(m_Renderer.GetVulkanDevice(), m_DepthStencilImage, m_DepthStencilImageMemory, 0);

        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image           = m_DepthStencilImage;
        imageViewCreateInfo.viewType        = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format          = m_DepthStencilFormat;
        imageViewCreateInfo.components.r    = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g    = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b    = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a    = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT | (m_StencilAvailable ? VK_IMAGE_ASPECT_STENCIL_BIT : 0);
        imageViewCreateInfo.subresourceRange.baseMipLevel   = 0;
        imageViewCreateInfo.subresourceRange.levelCount     = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount     = 1;

        VulkanDebug::CheckResult(vkCreateImageView(m_Renderer.GetVulkanDevice(), &imageViewCreateInfo, nullptr, &m_DepthStencilImageView), "Image view");
    }

    void VulkanGraphicContext::DeinitDepthStencilImage()
    {
        vkDestroyImageView(m_Renderer.GetVulkanDevice(), m_DepthStencilImageView, nullptr);
        vkFreeMemory(m_Renderer.GetVulkanDevice(), m_DepthStencilImageMemory, nullptr);
        vkDestroyImage(m_Renderer.GetVulkanDevice(), m_DepthStencilImage, nullptr);
    }

    void VulkanGraphicContext::InitRenderPass()
    {
        std::array<VkAttachmentDescription, 2> attachments {};
        attachments[0].flags						= 0;
        attachments[0].format						= m_DepthStencilFormat;
        attachments[0].samples					    = VK_SAMPLE_COUNT_1_BIT;
        attachments[0].loadOp						= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[0].storeOp					    = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[0].stencilLoadOp				= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[0].stencilStoreOp				= VK_ATTACHMENT_STORE_OP_STORE;
        attachments[0].initialLayout				= VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[0].finalLayout				    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        attachments[1].flags						= 0;
        attachments[1].format						= m_SurfaceFormat.format;
        attachments[1].samples					    = VK_SAMPLE_COUNT_1_BIT;
        attachments[1].loadOp						= VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[1].storeOp					    = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[1].initialLayout				= VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[1].finalLayout				    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;


        VkAttachmentReference sub_pass_0_depth_stencil_attachment {};
        sub_pass_0_depth_stencil_attachment.attachment	= 0;
        sub_pass_0_depth_stencil_attachment.layout		= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        std::array<VkAttachmentReference, 1> sub_pass_0_color_attachments {};
        sub_pass_0_color_attachments[ 0 ].attachment	= 1;
        sub_pass_0_color_attachments[ 0 ].layout		= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        std::array<VkSubpassDescription, 1> sub_passes {};
        sub_passes[0].pipelineBindPoint			= VK_PIPELINE_BIND_POINT_GRAPHICS;
        sub_passes[0].colorAttachmentCount		= sub_pass_0_color_attachments.size();
        sub_passes[0].pColorAttachments			= sub_pass_0_color_attachments.data();		// layout(location=0) out vec4 FinalColor;
        sub_passes[0].pDepthStencilAttachment		= &sub_pass_0_depth_stencil_attachment;


        VkRenderPassCreateInfo render_pass_create_info {};
        render_pass_create_info.sType				= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_create_info.attachmentCount		= attachments.size();
        render_pass_create_info.pAttachments		= attachments.data();
        render_pass_create_info.subpassCount		= sub_passes.size();
        render_pass_create_info.pSubpasses			= sub_passes.data();

        VulkanDebug::CheckResult(vkCreateRenderPass(m_Renderer.GetVulkanDevice(), &render_pass_create_info, nullptr, &m_RenderPass), "Renderpass creation failed");
    }

    void VulkanGraphicContext::DeinitRenderPass()
    {
        vkDestroyRenderPass(m_Renderer.GetVulkanDevice(), m_RenderPass, nullptr);
    }

    void VulkanGraphicContext::InitFramebuffers()
    {
        m_Framebuffers.resize(m_SwapchainImageCount);
        for (uint32_t i = 0; i < m_SwapchainImageCount; i++)
        {
            std::array<VkImageView, 2> attachments {};
            attachments[0] = m_DepthStencilImageView;
            attachments[1] = m_SwapchainImageViews[i];

            VkFramebufferCreateInfo framebufferCreateInfo{};
            framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferCreateInfo.renderPass = m_RenderPass;
            framebufferCreateInfo.attachmentCount = attachments.size();
            framebufferCreateInfo.pAttachments = attachments.data();
            framebufferCreateInfo.width     = m_SurfaceWidth;
            framebufferCreateInfo.height    = m_SurfaceHeight;
            framebufferCreateInfo.layers    = 1;
            VulkanDebug::CheckResult(vkCreateFramebuffer(m_Renderer.GetVulkanDevice(), &framebufferCreateInfo, nullptr, &m_Framebuffers[i]));
        }
    }

    void VulkanGraphicContext::DeinitFramebuffers()
    {
        for (auto& f : m_Framebuffers)
            vkDestroyFramebuffer(m_Renderer.GetVulkanDevice(), f, nullptr);
    }
}

int main()
{
    glfwInit();
    int width = 800;
    int height = 600;
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(width, height, "Eternity", nullptr, nullptr);

    Eternity::VulkanRenderer r;
    Eternity::VulkanGraphicContext context(r, window);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }

    glfwTerminate();
    return VK_SUCCESS;
}