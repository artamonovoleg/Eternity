#include <iostream>
#include <mutex>
#include <vector>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "VulkanHelper.hpp"

namespace 
{
    GLFWwindow* pWindow;
    std::once_flag glfwInitFlag;
}

bool CreateWindow(int width, int height, const std::string& title)
{
    std::call_once(glfwInitFlag, [] () { glfwInit(); });
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
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

class VulkanRenderer
{
    private:
        VkInstance                  m_Instance          = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT    m_DebugMessenger    = VK_NULL_HANDLE;
        VkPhysicalDevice            m_GPU               = VK_NULL_HANDLE;
        VkDevice                    m_Device            = VK_NULL_HANDLE;
        VkSurfaceKHR                m_Surface           = VK_NULL_HANDLE;

        void InitInstance();
    public:
        void InitVulkan();
        void DeinitVulkan();
};

void VulkanRenderer::InitVulkan()
{
    InitInstance();
}

void VulkanRenderer::DeinitVulkan()
{
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

int main(int, char **) 
{
    CreateWindow(800, 600, "Eternity");

    VulkanRenderer renderer;
    renderer.InitVulkan();

    while (!glfwWindowShouldClose(GetCurrentWindow()))
    {
        glfwPollEvents();
    }

    renderer.DeinitVulkan();

    DestroyWindow();
    return 0; 
}