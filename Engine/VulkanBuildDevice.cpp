#include <set>
#include "VulkanHelper.hpp"

namespace vkh
{
    const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    QueueFamilyIndices                              FindQueueFamilies(const VkPhysicalDevice& device, const VkSurfaceKHR& surface) 
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

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

            if (presentSupport)
                indices.presentFamily = i;

            if (indices.isComplete())
                break;

            i++;
        }

        return indices;
    }

    bool CheckDeviceExtensionSupport(const VkPhysicalDevice& device) 
    {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : availableExtensions) 
            requiredExtensions.erase(extension.extensionName);

        return requiredExtensions.empty();
    }

    bool                                            IsDeviceSuitable(const VkPhysicalDevice& device, const VkSurfaceKHR& surface) 
    {
        QueueFamilyIndices indices = FindQueueFamilies(device, surface);

        bool extensionsSupported = CheckDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapchainSupportDetails swapChainSupport = QuerySwapchainSupport(device, surface);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        return indices.isComplete() && extensionsSupported && swapChainAdequate;
    }

    VkPhysicalDevice                                SelectPhysicalDevice(const VkInstance& instance, const VkSurfaceKHR& surface)
    {
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        for (const auto& device : devices) 
        {
            if (IsDeviceSuitable(device, surface)) 
            {
                physicalDevice = device;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE) 
            throw std::runtime_error("failed to find a suitable GPU!");

        return physicalDevice;
    }

    VkDevice                                        BuildDevice(const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& surface)
    {
        VkDevice                device  = VK_NULL_HANDLE;

        vkh::QueueFamilyIndices indices = vkh::FindQueueFamilies(physicalDevice, surface);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) 
        {
            VkDeviceQueueCreateInfo queueCI
            {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = queueFamily,
                .queueCount = 1,
                .pQueuePriorities = &queuePriority
            };
            queueCreateInfos.push_back(queueCI);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};

        VkDeviceCreateInfo deviceCI
        {
            .sType                      = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .queueCreateInfoCount       = static_cast<uint32_t>(queueCreateInfos.size()),
            .pQueueCreateInfos          = queueCreateInfos.data(),
            .enabledExtensionCount      = static_cast<uint32_t>(deviceExtensions.size()),
            .ppEnabledExtensionNames    = deviceExtensions.data(),
            .pEnabledFeatures           = &deviceFeatures,
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