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

    VkDebugUtilsMessengerCreateInfoEXT* GetDebugCreateInfo();
    void CreateDebugMessenger(const VkInstance& instance);
    void DestroyDebugMessenger(const VkInstance& instance);
};
