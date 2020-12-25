//
// Created by artamonovoleg on 24.12.2020.
//

#pragma once
#include "VulkanHelper.hpp"
#include "Base.hpp"

class VulkanInstance
{
    private:
        VkInstance m_Instance;
    public:
        VulkanInstance()
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

        ~VulkanInstance()
        {
            vkDestroyInstance(m_Instance, nullptr);
        }
};

