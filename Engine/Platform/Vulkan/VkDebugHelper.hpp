//
// Created by artamonovoleg on 20.12.2020.
//

#pragma once
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

namespace VkDebugHelper
{
    void CheckResult(VkResult result, const std::string& msg = "");
    void SetupDebug(std::vector<const char*>& instanceLayers, std::vector<const char*>& instanceExtensions, std::vector<const char*>& deviceLayers);
    void InitDebug(VkInstance& instance);
    void DeinitDebug(VkInstance& instance);
    VkDebugReportCallbackCreateInfoEXT* GetDebugReportCallbackInfo();
};
