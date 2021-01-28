#include <vector>
#include <cstring>
#include <GLFW/glfw3.h>
#include "Instance.hpp"
#include "Base.hpp"
#include "VkCheck.hpp"

//
const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

#ifdef ET_DEBUG
    const bool enableValidationLayers = true;
#else
    const bool enableValidationLayers = false;
#endif
//

namespace Eternity
{
    Instance::Instance()
    {
        CreateInstance();
        SetupDebugMessenger();
        ET_TRACE("Instance created");
    }

    Instance::~Instance()
    {
        if (enableValidationLayers)
            DestroyDebugUtilsMessengerEXT();
        vkDestroyInstance(m_Instance, nullptr);
        ET_TRACE("Instance destroyed");
    }

    void Instance::CreateInstance()
    {
        if (enableValidationLayers && !CheckValidationLayersSupport())
            ET_ASSERT(false, "Validation layers requested but not supported");
        
        VkApplicationInfo appInfo
        {
            .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName   = "Eternity",
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName        = "Eternity",
            .engineVersion      = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion         = VK_API_VERSION_1_2
        };


        VkInstanceCreateInfo    instanceCI{};
        instanceCI.sType                    = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCI.pApplicationInfo         = &appInfo;

        auto extensions = GetExtensions();
        instanceCI.enabledExtensionCount    = static_cast<uint32_t>(extensions.size());
        instanceCI.ppEnabledExtensionNames  = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCI;
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

        VkCheck(vkCreateInstance(&instanceCI, nullptr, &m_Instance));
    }

    bool Instance::ValidationLayersEnabled()
    {
        return enableValidationLayers;
    }

    const std::vector<const char*>& Instance::GetLayers()
    {
        return validationLayers;
    }

    std::vector<const char*> Instance::GetExtensions()
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers)
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        return extensions;
    }

    /// Validation layers
    bool Instance::CheckValidationLayersSupport()
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers) 
        {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (std::strcmp(layerName, layerProperties.layerName) == 0) 
                {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) 
    {
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
            ET_TRACE("[ Validation layer ]", pCallbackData->pMessage);
        else
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT )
            ET_INFO("[ Validation layer ]", pCallbackData->pMessage);
        else
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            ET_WARN("[ Validation layer ]", pCallbackData->pMessage);
        else
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
            ET_ERROR("[ Validation layer ]", pCallbackData->pMessage);
        return VK_FALSE;
    }

    void Instance::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) 
    {
        createInfo = {};
        createInfo.sType            = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity  = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType      = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback  = DebugCallback;
    }
    
    VkResult Instance::CreateDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo)
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(m_Instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) 
            return func(m_Instance, pCreateInfo, nullptr, &m_DebugMessenger);
        else
            return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
    
    void Instance::DestroyDebugUtilsMessengerEXT()
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
            func(m_Instance, m_DebugMessenger, nullptr);
    }

    void Instance::SetupDebugMessenger()
    {
        if (!enableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        PopulateDebugMessengerCreateInfo(createInfo);

        VkCheck(CreateDebugUtilsMessengerEXT(&createInfo));
    }

} // namespace Eternity
