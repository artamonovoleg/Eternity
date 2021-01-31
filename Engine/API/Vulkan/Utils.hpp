#pragma once
#include <vulkan/vulkan.h>

namespace Eternity
{
    class PhysicalDevice;

    uint32_t FindMemoryType(const PhysicalDevice& physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
    VkFormat FindDepthFormat(const PhysicalDevice& physicalDevice);
} // namespace Eternity
