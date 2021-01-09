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
    struct QueueFamilyIndices 
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() 
        {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapchainSupportDetails 
    {
        VkSurfaceCapabilitiesKHR        capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR>   presentModes;
    };

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

    QueueFamilyIndices                              FindQueueFamilies(const VkPhysicalDevice& device, const VkSurfaceKHR& surface);
    VkPhysicalDevice                                SelectPhysicalDevice(const VkInstance& instance, const VkSurfaceKHR& surface);
    VkDevice                                        BuildDevice(const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& surface);

    VkSurfaceFormatKHR                              ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR                                ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D                                      ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    SwapchainSupportDetails                         QuerySwapchainSupport(const VkPhysicalDevice& device, const VkSurfaceKHR& surface);
    VkSwapchainKHR                                  BuildSwapchain(const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& surface, const VkDevice& device);
}