//
// Created by artamonovoleg on 20.12.2020.
//

#pragma once
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

namespace VulkanHelper
{
    std::vector<const char*> GetRequiredExtensions();
    std::vector<const char*> GetInstanceLayers();
    void SetupDebugMessenger();
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
};
