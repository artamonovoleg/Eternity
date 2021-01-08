#include "VulkanHelper.hpp"

namespace vkh
{
    QueueFamilyIndices                              FindQueueFamilies(VkPhysicalDevice device) 
    {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) 
        {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                indices.graphicsFamily = i;

            if (indices.isComplete())
                break;

            i++;
        }

        return indices;
    }

    bool IsDeviceSuitable(VkPhysicalDevice device) 
    {
        QueueFamilyIndices indices = FindQueueFamilies(device);

        return indices.graphicsFamily.has_value();
    }

    VkPhysicalDevice                                SelectPhysicalDevice(const VkInstance& instance)
    {
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        for (const auto& device : devices) 
        {
            if (IsDeviceSuitable(device)) 
            {
                physicalDevice = device;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE) 
            throw std::runtime_error("failed to find a suitable GPU!");

        return physicalDevice;
    }

    VkDevice                                        BuildDevice(const VkPhysicalDevice& physicalDevice)
    {
        VkDevice                device  = VK_NULL_HANDLE;

        vkh::QueueFamilyIndices indices = vkh::FindQueueFamilies(physicalDevice);

        float queuePriority = 1.0f;
        VkDeviceQueueCreateInfo queueCI
        {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = indices.graphicsFamily.value(),
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
        };

        VkPhysicalDeviceFeatures deviceFeatures{};

        VkDeviceCreateInfo deviceCI
        {
            .sType                  = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .queueCreateInfoCount   = 1,
            .pQueueCreateInfos      = &queueCI,
            .enabledExtensionCount  = 0,
            .pEnabledFeatures       = &deviceFeatures,
        };

        const auto layers                 = vkh::GetValidationLayers();
        if (vkh::IsVulkanDebugEnabled())
        {

            deviceCI.enabledLayerCount      = static_cast<uint32_t>(layers.size());
            deviceCI.ppEnabledLayerNames    = layers.data();
        }
        else
        {
            deviceCI.enabledLayerCount      = 0;
        }

        auto res = vkCreateDevice(physicalDevice, &deviceCI, nullptr, &device);
        vkh::Check(res, "Device create failed");

        return device;
    }
}