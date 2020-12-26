//
// Created by artamonovoleg on 24.12.2020.
//

#include <GLFW/glfw3.h>
#include "InstanceBuilder.hpp"
#include "Base.hpp"
#include "VulkanCheck.hpp"

namespace vkb
{
    void Instance::Destroy() const
    {
        vkb::DestroyDebugUtilsMessenger(instance, debugMessenger, nullptr);
        vkDestroyInstance(instance, nullptr);
    }

    /// Debuging
    static VKAPI_ATTR VkBool32 VKAPI_CALL DefaultDebugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData)
    {
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
            ET_CORE_TRACE(pCallbackData->pMessageIdName, pCallbackData->pMessage);
        else
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
            ET_CORE_WARN(pCallbackData->pMessage);
        else
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
            ET_CORE_ERROR(pCallbackData->pMessage);
        return VK_FALSE;
    }

    void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
    {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = DefaultDebugCallback;
    }

    VkResult CreateDebugUtilsMessenger(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        else
            return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    void DestroyDebugUtilsMessenger(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
            func(instance, debugMessenger, pAllocator);
    }
    ///

    void InstanceBuilder::SetupRequiredExtensionsAndLayers()
    {
        uint32_t extensionsCount;
        const char **glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionsCount);
        m_IntanceExtensions.insert(m_IntanceExtensions.begin(), glfwExtensions, glfwExtensions + extensionsCount);
        if (m_DebugEnabled)
        {
            m_IntanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            m_InstanceLayers.push_back("VK_LAYER_KHRONOS_validation");
        }
    }

    void InstanceBuilder::SetAppName(const std::string &name)
    {
        m_AppInfo.pApplicationName = name.data();
    }

    void InstanceBuilder::SetAppVersion(uint32_t appVersion)
    {
        m_AppInfo.applicationVersion = appVersion;
    }

    void InstanceBuilder::SetAppVersion(int minor, int major, int patch)
    {
        m_AppInfo.applicationVersion = VK_MAKE_VERSION(minor, major, patch);
    }

    void InstanceBuilder::SetEngineName(const std::string &engineName)
    {
        m_AppInfo.pEngineName = engineName.data();
    }

    void InstanceBuilder::SetEngineVersion(uint32_t engineVersion)
    {
        m_AppInfo.engineVersion = engineVersion;
    }

    void InstanceBuilder::SetEngineVersion(int minor, int major, int patch)
    {
        m_AppInfo.engineVersion = VK_MAKE_VERSION(minor, major, patch);
    }

    void InstanceBuilder::RequireAPIVersion(int major, int minor, int patch)
    {
        m_AppInfo.apiVersion  = VK_MAKE_VERSION(major, minor, patch);
    }

    void InstanceBuilder::RequireAPIVersion(uint32_t version)
    {
        m_AppInfo.apiVersion = version;
    }

    void InstanceBuilder::RequestDebug()
    {
        m_DebugEnabled = true;
//        PopulateDebugMessengerCreateInfo(debugCreateInfo);
    }

    void InstanceBuilder::Build()
    {
        SetupRequiredExtensionsAndLayers();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (m_DebugEnabled)
            PopulateDebugMessengerCreateInfo(debugCreateInfo);

        VkInstanceCreateInfo instanceCreateInfo{};
        instanceCreateInfo.sType                    = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pApplicationInfo         = &m_AppInfo;
        instanceCreateInfo.enabledLayerCount        = m_InstanceLayers.size();
        instanceCreateInfo.ppEnabledLayerNames      = m_InstanceLayers.data();
        instanceCreateInfo.enabledExtensionCount    = m_IntanceExtensions.size();
        instanceCreateInfo.ppEnabledExtensionNames  = m_IntanceExtensions.data();
        instanceCreateInfo.pNext                    = &debugCreateInfo;
        vkb::Check(vkCreateInstance(&instanceCreateInfo, nullptr, &m_Instance), "Instance create");

        VkDebugUtilsMessengerEXT debugMessenger{};

        if (m_DebugEnabled)
            CreateDebugUtilsMessenger(m_Instance, &debugCreateInfo, nullptr, &debugMessenger);

        m_Result.instance       = m_Instance;
        m_Result.debugMessenger = debugMessenger;
        m_Result.layers         = m_InstanceLayers;
    }

    Instance InstanceBuilder::Get() const
    {
        return m_Result;
    }
}