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

        void InitInstance();
        void CreateDevice();
    public:
        void InitVulkan();
        void DeinitVulkan();
};

void VulkanRenderer::InitVulkan()
{
    InitInstance();
    CreateDevice();
}

void VulkanRenderer::DeinitVulkan()
{
    vkDestroyDevice(m_Device, nullptr);
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

    auto inst_ret = vkh::BuildInstance(appInfo, true);
    m_Instance          = inst_ret.first;
    m_DebugMessenger    = inst_ret.second;
}

void VulkanRenderer::CreateDevice()
{
    m_GPU       = vkh::SelectPhysicalDevice(m_Instance);
    m_Device    = vkh::BuildDevice(m_GPU);
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