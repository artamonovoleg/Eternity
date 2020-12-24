#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdint>

#include "Base.hpp"
#include "VulkanHelper.hpp"




namespace Eternity
{
    class VulkanRenderer
    {
        private:
            VkInstance          m_Instance          = VK_NULL_HANDLE;

            VkPhysicalDevice    m_PhysicalDevice    = VK_NULL_HANDLE;

            void CreateInstance();
            void DestroyInstance();

            void PickPhysicalDevice();
            void CreateLogicalDevice();
            void DestroyLogicalDevice();
        public:
            VulkanRenderer();
            ~VulkanRenderer();
    };

    VulkanRenderer::VulkanRenderer()
    {
        CreateInstance();
        VulkanHelper::CreateDebugMessenger(m_Instance);
        PickPhysicalDevice();
    }

    VulkanRenderer::~VulkanRenderer()
    {
        VulkanHelper::DestroyDebugMessenger(m_Instance);
        DestroyInstance();
    }




    void VulkanRenderer::CreateInstance()
    {
        VkApplicationInfo appInfo{};
        appInfo.sType               = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName    = "Hello Triangle";
        appInfo.applicationVersion  = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName         = "No Engine";
        appInfo.engineVersion       = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion          = VK_API_VERSION_1_0;

        VkInstanceCreateInfo instanceCreateInfo{};
        instanceCreateInfo.sType                    = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pApplicationInfo         = &appInfo;

        auto extensions  = VulkanHelper::GetRequiredExtensions();
        instanceCreateInfo.enabledExtensionCount    = extensions.size();
        instanceCreateInfo.ppEnabledExtensionNames  = extensions.data();

        auto layers = VulkanHelper::GetInstanceLayers();
        instanceCreateInfo.enabledLayerCount        = layers.size();
        instanceCreateInfo.ppEnabledLayerNames      = layers.data();
        instanceCreateInfo.pNext                    = VulkanHelper::GetDebugCreateInfo();
        ET_CORE_ASSERT(vkCreateInstance(&instanceCreateInfo, nullptr, &m_Instance) == VK_SUCCESS, "Instance creation");
    }

    void VulkanRenderer::DestroyInstance()
    {
        vkDestroyInstance(m_Instance, nullptr);
    }

    void VulkanRenderer::PickPhysicalDevice()
    {

    }

    void VulkanRenderer::CreateLogicalDevice()
    {
    }

    void VulkanRenderer::DestroyLogicalDevice()
    {

    }
}

int main()
{
    Eternity::VulkanRenderer renderer;

    return EXIT_SUCCESS;
}

