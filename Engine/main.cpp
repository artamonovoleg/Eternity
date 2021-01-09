#include <iostream>
#include <vector>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "Window.hpp"
#include "VulkanHelper.hpp"

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

        void InitInstance();
        void CreateSurface();
        void CreateDevice();
        void CreateSwapchain();
        void InitCommands();
    public:
        void InitVulkan();
        void DeinitVulkan();
};

void VulkanRenderer::InitVulkan()
{
    InitInstance();
    CreateSurface();
    CreateDevice();
    CreateSwapchain();
    InitCommands();
}

void VulkanRenderer::DeinitVulkan()
{
    vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
    for (const auto& imageView : m_SwapchainImageViews)
        vkDestroyImageView(m_Device, imageView, nullptr);
    vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);
    vkDestroyDevice(m_Device, nullptr);
    vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
    vkh::DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
    vkDestroyInstance(m_Instance, nullptr);
}

void VulkanRenderer::InitInstance()
{
    VkApplicationInfo appInfo
    {
        .sType          = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pEngineName    = "Eternity",
        .engineVersion  = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion     = VK_API_VERSION_1_2
    };

    auto inst_ret       = vkh::BuildInstance(appInfo, true);
    m_Instance          = inst_ret.first;
    m_DebugMessenger    = inst_ret.second;
}

void VulkanRenderer::CreateSurface()
{
    auto res = glfwCreateWindowSurface(m_Instance, Eternity::GetCurrentWindow(), nullptr, &m_Surface);
    vkh::Check(res, "Surface create failed");
}

void VulkanRenderer::CreateDevice()
{
    m_GPU       = vkh::SelectPhysicalDevice(m_Instance, m_Surface);
    m_Device    = vkh::BuildDevice(m_GPU, m_Surface);

    auto indices = vkh::FindQueueFamilies(m_GPU, m_Surface);
    m_GraphicsQueueFamily = indices.graphicsFamily.value();
    m_PresentQueueFamily = indices.presentFamily.value();
    vkGetDeviceQueue(m_Device, m_GraphicsQueueFamily, 0, &m_GraphicsQueue);
    vkGetDeviceQueue(m_Device, m_PresentQueueFamily, 0, &m_PresentQueue);
}

void VulkanRenderer::CreateSwapchain()
{
    auto swachain_ret = vkh::BuildSwapchain(m_GPU, m_Surface, m_Device);
    m_Swapchain = swachain_ret.swapchain;

    m_SwapchainImages                   = swachain_ret.images;
    m_SwapchainImageFormat              = swachain_ret.imageFormat;
    m_SwapchainExtent                   = swachain_ret.extent;
    m_SwapchainImageViews               = swachain_ret.imageViews;
}

void VulkanRenderer::InitCommands()
{
    VkCommandPoolCreateInfo commandPoolCI
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        //we also want the pool to allow for resetting of individual command buffers
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        //the command pool will be one that can submit graphics commands
        .queueFamilyIndex = m_GraphicsQueueFamily
    };

	auto res = vkCreateCommandPool(m_Device, &commandPoolCI, nullptr, &m_CommandPool);
    vkh::Check(res, "Command pool create failed");

    VkCommandBufferAllocateInfo cmdAllocCI
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        //commands will be made from our commandPool
        .commandPool = m_CommandPool,
        // command level is Primary
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        //we will allocate 1 command buffer
        .commandBufferCount = 1
    };

    res = vkAllocateCommandBuffers(m_Device, &cmdAllocCI, &m_CommandBuffer);
    vkh::Check(res, "Command buffer allocate failed");
}

int main(int, char **) 
{
    Eternity::CreateWindow(800, 600, "Eternity");

    VulkanRenderer renderer;
    renderer.InitVulkan();

    while (!glfwWindowShouldClose(Eternity::GetCurrentWindow()))
    {
        glfwPollEvents();
    }

    renderer.DeinitVulkan();

    Eternity::DestroyWindow();
    return 0; 
}