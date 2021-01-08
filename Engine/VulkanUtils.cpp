#include "VulkanHelper.hpp"

namespace vkh
{
    void Check(VkResult result, const std::string& msg)
    {
        // TODO: write this function
    }

    std::vector<const char*> GetRequiredExtensions() 
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (IsVulkanDebugEnabled()) 
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        return extensions;
    }
}