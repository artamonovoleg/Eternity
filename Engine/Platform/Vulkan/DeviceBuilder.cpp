//
// Created by artamonovoleg on 26.12.2020.
//

#include <set>
#include "DeviceBuilder.hpp"
#include "VulkanCheck.hpp"

namespace vkb
{

    void DeviceBuilder::FindQueueFamilies()
    {
        uint32_t queueFamilyCount = 0;

        vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice.physicalDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice.physicalDevice, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        bool graphicsFound  = false;
        bool presentFound   = false;
        for (const auto& queueFamily : queueFamilies)
        {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                m_GraphicsQueueFamily = i;
                graphicsFound = true;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(m_PhysicalDevice.physicalDevice, i, m_Surface, &presentSupport);

            if (presentSupport)
            {
                m_PresentQueueFamily    = i;
                presentFound            = true;
            }

            if (graphicsFound && presentFound)
                break;
            i++;
        }
    }

    void DeviceBuilder::SetSurface(VkSurfaceKHR surface)
    {
        m_Surface = surface;
    }

    void DeviceBuilder::Build()
    {
        FindQueueFamilies();

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { m_PresentQueueFamily, m_GraphicsQueueFamily };

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        // now dont need any features
        VkPhysicalDeviceFeatures deviceFeatures{};

        VkDeviceCreateInfo deviceCreateInfo{};
        deviceCreateInfo.sType                      = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.queueCreateInfoCount       = static_cast<uint32_t>(queueCreateInfos.size());
        deviceCreateInfo.pQueueCreateInfos          = queueCreateInfos.data();
        deviceCreateInfo.pEnabledFeatures           = &deviceFeatures;
        const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
        deviceCreateInfo.enabledExtensionCount      = static_cast<uint32_t>(deviceExtensions.size()); // declared on top of file
        deviceCreateInfo.ppEnabledExtensionNames    = deviceExtensions.data();
        deviceCreateInfo.enabledLayerCount          = static_cast<uint32_t>(m_Instance.layers.size());
        deviceCreateInfo.ppEnabledLayerNames        = m_Instance.layers.data();

        vkb::Check(vkCreateDevice(m_PhysicalDevice.physicalDevice, &deviceCreateInfo, nullptr, &m_Device.device), "Create device");
        m_Device.graphicsQueueFamilyIndex   = m_GraphicsQueueFamily;
        m_Device.presentQueueFamilyIndex    = m_PresentQueueFamily;
    }

    vkb::Device DeviceBuilder::Get() const
    {
        return m_Device;
    }
}