#pragma once
#include <iostream>
#include <tuple>
#include <string>
#include <vector>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

namespace vkh
{
    static const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
    static bool enableValidationLayers = false;

    void Check(VkResult result, const std::string& msg = "")
    {
    }

    std::vector<const char*> GetRequiredExtensions() 
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) 
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        return extensions;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData) 
    {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) 
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        else
            return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) 
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
            func(instance, debugMessenger, pAllocator);
    }

    void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& CI) 
    {
        CI = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = DebugCallback
        };
    }

    void SetupDebugMessenger(const VkInstance& instance, VkDebugUtilsMessengerEXT& debugMessenger)
    {
        if (!enableValidationLayers) return;
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        PopulateDebugMessengerCreateInfo(createInfo);

        CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger);
    }

    std::pair<VkInstance, VkDebugUtilsMessengerEXT> BuildInstance(const VkApplicationInfo& appInfo, bool debugEnabled)
    {
        enableValidationLayers = debugEnabled;

        VkInstance instance                     = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;

        VkInstanceCreateInfo instanceCI
        {
            .sType              = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo   = &appInfo
        };

        auto extensions = GetRequiredExtensions();
        instanceCI.enabledExtensionCount    = static_cast<uint32_t>(extensions.size());
        instanceCI.ppEnabledExtensionNames  = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCI{};
        if (enableValidationLayers)
        {
            instanceCI.enabledLayerCount    = static_cast<uint32_t>(validationLayers.size());
            instanceCI.ppEnabledLayerNames  = validationLayers.data();

            PopulateDebugMessengerCreateInfo(debugCI);
            instanceCI.pNext                = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCI;
        }
        else
        {
            instanceCI.enabledLayerCount = 0;
            
            instanceCI.pNext = nullptr;
        }

        auto res = vkCreateInstance(&instanceCI, nullptr, &instance); 
        Check(res, "Create instance failed");

        if (enableValidationLayers)
            SetupDebugMessenger(instance, debugMessenger);
            
        return std::make_pair(instance, debugMessenger);
    }
}