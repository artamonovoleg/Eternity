#pragma once
#include <vulkan/vulkan.h>
#include "Base.hpp"

namespace Eternity
{
    static void VkCheck(VkResult result)
    {
        ET_ASSERT(result >= VK_SUCCESS);
    }
}