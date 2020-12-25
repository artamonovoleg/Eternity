//
// Created by artamonovoleg on 25.12.2020.
//

#pragma once
#include <string>
#include <vulkan/vulkan.h>

namespace vkb
{
    void Check(VkResult result, const std::string& msg = "");
}
