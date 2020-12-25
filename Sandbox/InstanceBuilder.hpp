//
// Created by artamonovoleg on 24.12.2020.
//

#pragma once
#include <vulkan/vulkan.h>
#include <string>
#include <vector>

namespace vkb
{
    /// Debugging
    void DestroyDebugUtilsMessenger(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
    ///
    class Instance;
    class InstanceBuilder
    {
        private:
            bool m_DebugEnabled = false;
            std::vector<const char*>    m_InstanceLayers;
            std::vector<const char*>    m_IntanceExtensions;
            VkApplicationInfo           m_AppInfo       = {};
            VkInstance                  m_Instance      = VK_NULL_HANDLE;

            void SetupRequiredExtensionsAndLayers();
        public:
            InstanceBuilder() = default;
            ~InstanceBuilder() = default;
            void SetAppName(const std::string& name);
            void SetAppVersion(uint32_t appVersion);
            void SetAppVersion(int minor, int major, int patch);
            void SetEngineName(const std::string& engineName);
            void SetEngineVersion(uint32_t engineVersion);
            void RequireAPIVersion(int minor, int major, int patch);
            void RequireAPIVersion(uint32_t version);
            void RequestDebug();
            void Build();
            Instance Get();
    };

    struct Instance
    {
            VkInstance                  m_Instance          = VK_NULL_HANDLE;
            VkDebugUtilsMessengerEXT    m_DebugMessenger    = VK_NULL_HANDLE;
            Instance() = default;
            explicit Instance(const VkInstance& instance, const VkDebugUtilsMessengerEXT& debugMessenger)
                : m_Instance(instance), m_DebugMessenger(debugMessenger) {}
            void Destroy() const
            {
                vkb::DestroyDebugUtilsMessenger(m_Instance, m_DebugMessenger, nullptr);
                vkDestroyInstance(m_Instance, nullptr);
            }
    };
}