#pragma once

#include <vulkan/vulkan.h>
#include "Base.hpp"

namespace Eternity
{
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData) 
    {
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
            ET_CORE_TRACE(pCallbackData->pMessage);
        else
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
            ET_CORE_INFO(pCallbackData->pMessage);
        else
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            ET_CORE_WARN(pCallbackData->pMessage);
        else
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
            ET_CORE_ERROR(pCallbackData->pMessage);

        return VK_FALSE;
    }
}