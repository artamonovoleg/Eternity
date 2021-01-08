#pragma once
#include <iostream>
#include <tuple>
#include <string>
#include <vector>
#include <optional>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

namespace vkh
{
    bool                                            IsVulkanDebugEnabled();

    void                                            Check(VkResult result, const std::string& msg = "");
    const std::vector<const char*>&                 GetValidationLayers();
    std::vector<const char*>                        GetRequiredExtensions();

    VkResult                                        CreateDebugUtilsMessengerEXT
    (VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    void                                            DestroyDebugUtilsMessengerEXT
    (VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
    /// Create instance funtion
    /// params: appInfo
    //          debugEnabled - VkDebugUtilsMessengerEXT creates only if true, if false = VK_NULL_HANDLE
    std::pair<VkInstance, VkDebugUtilsMessengerEXT> BuildInstance(const VkApplicationInfo& appInfo, bool debugEnabled);

    struct QueueFamilyIndices 
    {
        std::optional<uint32_t> graphicsFamily;

        bool isComplete() 
        {
            return graphicsFamily.has_value();
        }
    };

    QueueFamilyIndices                              FindQueueFamilies(VkPhysicalDevice device);
    VkPhysicalDevice                                SelectPhysicalDevice(const VkInstance& instance);
    VkDevice                                        BuildDevice(const VkPhysicalDevice& physicalDevice);
}