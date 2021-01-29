#include <set>
#include "Device.hpp"
#include "VkCheck.hpp"
#include "Instance.hpp"
#include "PhysicalDevice.hpp"

namespace Eternity
{
    Device::Device(const Instance& instance, const PhysicalDevice& physicalDevice)
        : m_PhysicalDevice(physicalDevice)
    {
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { m_PhysicalDevice.GetQueueFamilyIndex(QueueType::Graphics), m_PhysicalDevice.GetQueueFamilyIndex(QueueType::Present) };

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

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &deviceFeatures;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(m_PhysicalDevice.GetDeviceExtensions().size());
        createInfo.ppEnabledExtensionNames = m_PhysicalDevice.GetDeviceExtensions().data();

        if (instance.ValidationLayersEnabled())
        {
            createInfo.enabledLayerCount    = static_cast<uint32_t>(instance.GetLayers().size());
            createInfo.ppEnabledLayerNames  = instance.GetLayers().data();
        } 
        else
        {
            createInfo.enabledLayerCount = 0;
        }

        VkCheck(vkCreateDevice(physicalDevice, &createInfo, nullptr, &m_Device));
        ET_TRACE("Device created");

        vkGetDeviceQueue(m_Device, m_PhysicalDevice.GetQueueFamilyIndex(QueueType::Graphics), 0, &m_GraphicsQueue);
        vkGetDeviceQueue(m_Device, m_PhysicalDevice.GetQueueFamilyIndex(QueueType::Present), 0, &m_PresentQueue);
    }

    Device::~Device()
    {
        vkDestroyDevice(m_Device, nullptr);
        ET_TRACE("Device destroyed");
    }

    VkQueue Device::GetQueue(QueueType type)
    {
        switch (type)
        {
            case QueueType::Graphics:   return m_GraphicsQueue;
            case QueueType::Present:    return m_PresentQueue;
            default:                    return VK_NULL_HANDLE;
        }
    }
} // namespace Eternity

