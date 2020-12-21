//
// Created by artamonovoleg on 20.12.2020.
//

#include "VkDebugHelper.hpp"
#include "Assert.hpp"

namespace VkDebugHelper
{
#if ET_DEBUG
    void CheckResult(VkResult result, const std::string& msg)
    {
        if (result < 0)
        {
            switch (result)
            {
                case VK_ERROR_OUT_OF_HOST_MEMORY:
                    ET_CORE_ASSERT(false, "VK_ERROR_OUT_OF_HOST_MEMORY\t" + msg);
                    break;
                case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                    ET_CORE_ASSERT(false, "VK_ERROR_OUT_OF_DEVICE_MEMORY\t" + msg);
                    break;
                case VK_ERROR_INITIALIZATION_FAILED:
                    ET_CORE_ASSERT(false, "VK_ERROR_INITIALIZATION_FAILED\t" + msg);
                    break;
                case VK_ERROR_DEVICE_LOST:
                    ET_CORE_ASSERT(false, "VK_ERROR_DEVICE_LOST\t" + msg);
                    break;
                case VK_ERROR_MEMORY_MAP_FAILED:
                    ET_CORE_ASSERT(false, "VK_ERROR_MEMORY_MAP_FAILED\t" + msg);
                    break;
                case VK_ERROR_LAYER_NOT_PRESENT:
                    ET_CORE_ASSERT(false, "VK_ERROR_LAYER_NOT_PRESENT\t" + msg);
                    break;
                case VK_ERROR_EXTENSION_NOT_PRESENT:
                    ET_CORE_ASSERT(false, "VK_ERROR_EXTENSION_NOT_PRESENT\t" + msg);
                    break;
                case VK_ERROR_FEATURE_NOT_PRESENT:
                    ET_CORE_ASSERT(false, "VK_ERROR_FEATURE_NOT_PRESENT\t" + msg);
                    break;
                case VK_ERROR_INCOMPATIBLE_DRIVER:
                    ET_CORE_ASSERT(false, "VK_ERROR_INCOMPATIBLE_DRIVER\t" + msg);
                    break;
                case VK_ERROR_TOO_MANY_OBJECTS:
                    ET_CORE_ASSERT(false, "VK_ERROR_TOO_MANY_OBJECTS\t" + msg);
                    break;
                case VK_ERROR_FORMAT_NOT_SUPPORTED:
                    ET_CORE_ASSERT(false, "VK_ERROR_FORMAT_NOT_SUPPORTED\t" + msg);
                    break;
                case VK_ERROR_SURFACE_LOST_KHR:
                    ET_CORE_ASSERT(false, "VK_ERROR_SURFACE_LOST_KHR\t" + msg);
                    break;
                case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
                    ET_CORE_ASSERT(false, "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR\t" + msg);
                    break;
                case VK_SUBOPTIMAL_KHR:
                    ET_CORE_ASSERT(false, "VK_SUBOPTIMAL_KHR\t" + msg);
                    break;
                case VK_ERROR_OUT_OF_DATE_KHR:
                    ET_CORE_ASSERT(false, "VK_ERROR_OUT_OF_DATE_KHR\t" + msg);
                    break;
                case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
                    ET_CORE_ASSERT(false, "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR\t" + msg);
                    break;
                case VK_ERROR_VALIDATION_FAILED_EXT:
                    ET_CORE_ASSERT(false, "VK_ERROR_VALIDATION_FAILED_EXT\t" + msg);
                    break;
                default:
                    break;
            }
        }
    }

    VKAPI_ATTR VkBool32 VKAPI_CALL
    VulkanDebugCallback(VkDebugReportFlagsEXT       flags,
                        VkDebugReportObjectTypeEXT  objType,
                        uint64_t                    sourceObjs,
                        size_t                      location,
                        int32_t                     msgCode,
                        const char*                 layerPrefix,
                        const char*                 msg,
                        void*                       userData)
    {
        std::string log = "[ " + std::string(layerPrefix) + " ] " + msg;
        if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
            ET_CORE_INFO(log);
        if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
            ET_CORE_WARN(log);
        if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
            ET_CORE_PERFORMANCE(log);
        if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
            ET_CORE_ERROR(log);
        if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
            ET_CORE_TRACE(log);
        return VK_FALSE;
    }

    VkDebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfo{};
    VkDebugReportCallbackEXT           debugReport = VK_NULL_HANDLE;

    void SetupDebug(std::vector<const char*>& instanceLayers, std::vector<const char*>& instanceExtensions, std::vector<const char*>& deviceLayers)
    {
        debugReportCallbackCreateInfo.sType         = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        debugReportCallbackCreateInfo.pfnCallback   = VulkanDebugCallback;
        debugReportCallbackCreateInfo.flags         = VK_DEBUG_REPORT_INFORMATION_BIT_EXT         |
                                                        VK_DEBUG_REPORT_WARNING_BIT_EXT             |
                                                        VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
                                                        VK_DEBUG_REPORT_ERROR_BIT_EXT               |
                                                        VK_DEBUG_REPORT_DEBUG_BIT_EXT               |
                                                        0;

        instanceLayers.push_back("VK_LAYER_KHRONOS_validation");
        instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        deviceLayers.push_back("VK_LAYER_KHRONOS_validation");
    }

    PFN_vkCreateDebugReportCallbackEXT fVkCreateDebugReportCallbackEXT      = VK_NULL_HANDLE;
    PFN_vkDestroyDebugReportCallbackEXT fVkDestroyDebugReportCallbackEXT    = VK_NULL_HANDLE;

    void InitDebug(VkInstance& instance)
    {
        fVkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
        fVkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
        ET_CORE_ASSERT(fVkCreateDebugReportCallbackEXT != VK_NULL_HANDLE &&
                       fVkDestroyDebugReportCallbackEXT != VK_NULL_HANDLE, "Can't fetch debug function pointers");
        fVkCreateDebugReportCallbackEXT(instance, &debugReportCallbackCreateInfo, nullptr, &debugReport);
    }

    void DeinitDebug(VkInstance& instance)
    {
        fVkDestroyDebugReportCallbackEXT(instance, debugReport, nullptr);
    }

    VkDebugReportCallbackCreateInfoEXT* GetDebugReportCallbackInfo()
    {
        return &debugReportCallbackCreateInfo;
    };
#else
    void CheckResult(VkResult result){];
    void SetupDebug() {}
    void InitDebug() {}
    void DeinitDebug() {}
#endif
}