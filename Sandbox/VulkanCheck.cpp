//
// Created by artamonovoleg on 25.12.2020.
//

#include "VulkanCheck.hpp"
#include "Base.hpp"

#ifdef ET_DEBUG
void vkb::Check(VkResult result, const std::string& msg)
{
    if (result < 0)
    {
        switch(result)
        {
            case VK_ERROR_OUT_OF_HOST_MEMORY:
                ET_CORE_ERROR("VK_ERROR_OUT_OF_HOST_MEMORY", msg);
                break;
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                ET_CORE_ERROR("VK_ERROR_OUT_OF_DEVICE_MEMORY", msg);
                break;
            case VK_ERROR_INITIALIZATION_FAILED:
                ET_CORE_ERROR("VK_ERROR_INITIALIZATION_FAILED", msg);
                break;
            case VK_ERROR_DEVICE_LOST:
                ET_CORE_ERROR(VK_ERROR_DEVICE_LOST, msg);
                break;
            case VK_ERROR_MEMORY_MAP_FAILED:
                ET_CORE_ERROR("VK_ERROR_MEMORY_MAP_FAILED", msg);
                break;
            case VK_ERROR_LAYER_NOT_PRESENT:
                ET_CORE_ERROR("VK_ERROR_LAYER_NOT_PRESENT", msg);
                break;
            case VK_ERROR_EXTENSION_NOT_PRESENT:
                ET_CORE_ERROR("VK_ERROR_EXTENSION_NOT_PRESENT", msg);
                break;
            case VK_ERROR_FEATURE_NOT_PRESENT:
                ET_CORE_ERROR("VK_ERROR_FEATURE_NOT_PRESENT", msg);
                break;
            case VK_ERROR_INCOMPATIBLE_DRIVER:
                ET_CORE_ERROR("VK_ERROR_INCOMPATIBLE_DRIVER", msg);
                break;
            case VK_ERROR_TOO_MANY_OBJECTS:
                ET_CORE_ERROR("VK_ERROR_TOO_MANY_OBJECTS", msg);
                break;
            case VK_ERROR_FORMAT_NOT_SUPPORTED:
                ET_CORE_ERROR("VK_ERROR_FORMAT_NOT_SUPPORTED", msg);
                break;
            case VK_ERROR_SURFACE_LOST_KHR:
                ET_CORE_ERROR("VK_ERROR_SURFACE_LOST_KHR", msg);
                break;
            case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
                ET_CORE_ERROR("VK_ERROR_NATIVE_WINDOW_IN_USE_KHR", msg);
                break;
            case VK_SUBOPTIMAL_KHR:
                ET_CORE_ERROR("VK_SUBOPTIMAL_KHR", msg);
                break;
            case VK_ERROR_OUT_OF_DATE_KHR:
                ET_CORE_ERROR("VK_ERROR_OUT_OF_DATE_KHR", msg);
                break;
            case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
                ET_CORE_ERROR("VK_ERROR_INCOMPATIBLE_DISPLAY_KHR", msg);
                break;
            case VK_ERROR_VALIDATION_FAILED_EXT:
                ET_CORE_ERROR("VK_ERROR_VALIDATION_FAILED_EXT", msg);
                break;
            default:
                break;
        }
    }
}
#else
void vkb::Check(VkResult result, const std::string& msg) {}
#endif
