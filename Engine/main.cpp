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

        VkRenderPass                m_RenderPass            = VK_NULL_HANDLE;
        std::vector<VkFramebuffer>  m_Framebuffers          = {};

        // render loop
        VkSemaphore                 m_PresentSemaphore      = VK_NULL_HANDLE;
        
        void InitInstance();
        void CreateSurface();
        void CreateDevice();
        void CreateSwapchain();
        void InitCommands();
        void CreateRenderPass();
        void CreateFramebuffers();
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
    CreateRenderPass();
    CreateFramebuffers();
}

void VulkanRenderer::DeinitVulkan()
{
    for (const auto& framebuffer : m_Framebuffers)
        vkDestroyFramebuffer(m_Device, framebuffer, nullptr);
    vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);
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

void VulkanRenderer::CreateRenderPass()
{
    // the renderpass will use this color attachment.
	VkAttachmentDescription colorAttachment 
    {
	    // the attachment will have the format needed by the swapchain
	    .format         = m_SwapchainImageFormat,
	    // 1 sample, we won't be doing MSAA
	    .samples        = VK_SAMPLE_COUNT_1_BIT,
	    // Clear when this attachment is loaded
	    .loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR,
	    // keep the attachment stored when the renderpass ends
	    .storeOp        = VK_ATTACHMENT_STORE_OP_STORE,
	    // don't care about stencil
	    .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
	    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
	    // don't know or care about the starting layout of the attachment
	    .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
	    // after the renderpass ends, the image has to be on a layout ready for display
	    .finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    VkAttachmentReference coloAttachmentRef
    {
	    // attachment number will index into the pAttachments array in the parent renderpass itself
	    .attachment = 0,
	    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

	// create 1 subpass, which is the minimum you can do
	VkSubpassDescription subpass
    {
        .pipelineBindPoint      = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount   = 1,
        .pColorAttachments      = &coloAttachmentRef
    };

    VkRenderPassCreateInfo renderPassCI
    {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        //connect the color attachment to the info
        .attachmentCount = 1,
        .pAttachments = &colorAttachment,
        //connect the subpass to the info
        .subpassCount = 1,
        .pSubpasses = &subpass
    };
	
	auto res = vkCreateRenderPass(m_Device, &renderPassCI, nullptr, &m_RenderPass);
    vkh::Check(res, "RenderPass create failed");
}

void VulkanRenderer::CreateFramebuffers()
{
    // create the framebuffers for the swapchain images. This will connect the render-pass to the images for rendering
	VkFramebufferCreateInfo frambufferCI
    {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,

        .renderPass         = m_RenderPass,
        .attachmentCount    = 1,
        .width              = m_SwapchainExtent.width,
        .height             = m_SwapchainExtent.height,
        .layers             = 1
    };

	//grab how many images we have in the swapchain
	const uint32_t swapchainImageCount = m_SwapchainImages.size();
	m_Framebuffers = std::vector<VkFramebuffer>(swapchainImageCount);

	//create framebuffers for each of the swapchain image views
	for (int i = 0; i < swapchainImageCount; i++) 
    {
		frambufferCI.pAttachments = &m_SwapchainImageViews[i];
		vkh::Check(vkCreateFramebuffer(m_Device, &frambufferCI, nullptr, &m_Framebuffers[i]), "Framebuffer create failed");
	}
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