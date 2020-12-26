//
// Created by artamonovoleg on 26.12.2020.
//

#include <set>
#include "DeviceBuilder.hpp"
#include "VulkanCheck.hpp"
#include "Base.hpp"

namespace vkb
{

    uint32_t GetPresentationFamilyIndex(const vkb::PhysicalDevice& physicalDevice, std::vector<VkQueueFamilyProperties>& queueFamilies)
    {
        int i = 0;
        for (const auto& queueFamily : queueFamilies)
        {
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice.device, i, physicalDevice.surface, &presentSupport);
            if (presentSupport)
                return i;
            i++;
        }
        ET_CORE_ASSERT(false, "No presentation family");
        return -1;
    }

    uint32_t GetGraphicsFamilyIndex(const vkb::PhysicalDevice& physicalDevice, std::vector<VkQueueFamilyProperties>& queueFamilies)
    {
        int i = 0;
        for (const auto& queueFamily : queueFamilies)
        {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                return i;
            i++;
        }
        ET_CORE_ASSERT(false, "No graphics family");
        return -1;
    }

    uint32_t Device::GetQueueFamilyIndex(const vkb::PhysicalDevice& physicalDevice, QueueType type)
    {
        static uint32_t queueFamilyCount = 0;

        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice.device, &queueFamilyCount, nullptr);
        static std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice.device, &queueFamilyCount, queueFamilies.data());

        switch (type)
        {
            case QueueType::Graphics:
                return GetGraphicsFamilyIndex(physicalDevice, queueFamilies);
            case QueueType::Presentation:
                return GetPresentationFamilyIndex(physicalDevice, queueFamilies);
        }
        ET_CORE_ASSERT(false);
        return -1;
    }

    VkQueue Device::GetQueue(const vkb::PhysicalDevice& physicalDevice, vkb::QueueType type)
    {
        VkQueue result;
        vkGetDeviceQueue(device, GetQueueFamilyIndex(physicalDevice, type), 0, &result);
        return result;
    }

    void Device::Destroy() const
    {
        vkDestroyDevice(device, nullptr);
    }

    void DeviceBuilder::SetSurface(VkSurfaceKHR surface)
    {
        m_Surface = surface;
    }

    void DeviceBuilder::Build()
    {
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { m_Device.GetQueueFamilyIndex(m_PhysicalDevice, vkb::QueueType::Presentation),
                                                   m_Device.GetQueueFamilyIndex(m_PhysicalDevice, vkb::QueueType::Graphics)};

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

        vkb::Check(vkCreateDevice(m_PhysicalDevice.device, &deviceCreateInfo, nullptr, &m_Device.device), "Create device");
    }

    vkb::Device DeviceBuilder::Get() const
    {
        return m_Device;
    }
}