//
// Created by artamonovoleg on 20.12.2020.
//

#include "VkDebugHelper.hpp"
#include "Assert.hpp"

namespace VkDebugHelper
{
#if ET_DEBUG
    void CheckResult(VkResult result)
    {
        if (result < 0)
        {
            switch (result)
            {
                case VK_ERROR_OUT_OF_HOST_MEMORY:
                    ET_CORE_ASSERT(false, "VK_ERROR_OUT_OF_HOST_MEMORY");
                    break;
                case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                    ET_CORE_ASSERT(false, "VK_ERROR_OUT_OF_DEVICE_MEMORY");
                    break;
                case VK_ERROR_INITIALIZATION_FAILED:
                    ET_CORE_ASSERT(false, "VK_ERROR_INITIALIZATION_FAILED");
                    break;
                case VK_ERROR_DEVICE_LOST:
                    ET_CORE_ASSERT(false, "VK_ERROR_DEVICE_LOST");
                    break;
                case VK_ERROR_MEMORY_MAP_FAILED:
                    ET_CORE_ASSERT(false, "VK_ERROR_MEMORY_MAP_FAILED");
                    break;
                case VK_ERROR_LAYER_NOT_PRESENT:
                    ET_CORE_ASSERT(false, "VK_ERROR_LAYER_NOT_PRESENT");
                    break;
                case VK_ERROR_EXTENSION_NOT_PRESENT:
                    ET_CORE_ASSERT(false, "VK_ERROR_EXTENSION_NOT_PRESENT");
                    break;
                case VK_ERROR_FEATURE_NOT_PRESENT:
                    ET_CORE_ASSERT(false, "VK_ERROR_FEATURE_NOT_PRESENT");
                    break;
                case VK_ERROR_INCOMPATIBLE_DRIVER:
                    ET_CORE_ASSERT(false, "VK_ERROR_INCOMPATIBLE_DRIVER");
                    break;
                case VK_ERROR_TOO_MANY_OBJECTS:
                    ET_CORE_ASSERT(false, "VK_ERROR_TOO_MANY_OBJECTS");
                    break;
                case VK_ERROR_FORMAT_NOT_SUPPORTED:
                    ET_CORE_ASSERT(false, "VK_ERROR_FORMAT_NOT_SUPPORTED");
                    break;
                case VK_ERROR_SURFACE_LOST_KHR:
                    ET_CORE_ASSERT(false, "VK_ERROR_SURFACE_LOST_KHR");
                    break;
                case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
                    ET_CORE_ASSERT(false, "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR");
                    break;
                case VK_SUBOPTIMAL_KHR:
                    ET_CORE_ASSERT(false, "VK_SUBOPTIMAL_KHR");
                    break;
                case VK_ERROR_OUT_OF_DATE_KHR:
                    ET_CORE_ASSERT(false, "VK_ERROR_OUT_OF_DATE_KHR");
                    break;
                case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
                    ET_CORE_ASSERT(false, "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR");
                    break;
                case VK_ERROR_VALIDATION_FAILED_EXT:
                    ET_CORE_ASSERT(false, "VK_ERROR_VALIDATION_FAILED_EXT");
                    break;
            }
        }
    }
#else
inline void CheckResult(VkResult result){];
#endif
}