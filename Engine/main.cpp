#include <iostream>
#include <mutex>
#include <vector>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

namespace 
{
    GLFWwindow* pWindow;
    std::once_flag glfwInitFlag;
}

bool CreateWindow(int width, int height, const std::string& title)
{
    std::call_once(glfwInitFlag, [] () { glfwInit(); });
    pWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    return (pWindow != nullptr);
}

GLFWwindow* GetCurrentWindow()
{
    return pWindow;
}

void DestroyWindow()
{
    glfwDestroyWindow(pWindow);
    glfwTerminate();
}


class VulkanEngine
{
    private:
        VkInstance                  m_Instance          = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT    m_DebugMessenger    = VK_NULL_HANDLE;
        VkPhysicalDevice            m_GPU               = VK_NULL_HANDLE;
        VkDevice                    m_Device            = VK_NULL_HANDLE;
        VkSurfaceKHR                m_Surface           = VK_NULL_HANDLE;

        VkSwapchainKHR              m_Swapchain             = VK_NULL_HANDLE; // from other articles
        // image format expected by the windowing system
        VkFormat                    m_SwapchainImageFormat  = {}; 	
        //array of images from the swapchain
        std::vector<VkImage>        m_SwapchainImages       = {};
        //array of image-views from the swapchain
        std::vector<VkImageView>    m_SwapchainImageViews   = {};
    public:
        void InitVulkan();
        void InitSwapchain();
};

void VulkanEngine::InitVulkan()
{
}

int main(int, char **) 
{
    CreateWindow(800, 600, "Eternity");

    VulkanEngine engine;
    engine.InitVulkan();

    while (!glfwWindowShouldClose(GetCurrentWindow()))
    {
        glfwPollEvents();
    }

    DestroyWindow();
    return 0; 
}