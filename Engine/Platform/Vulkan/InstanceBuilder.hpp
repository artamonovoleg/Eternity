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

    struct Instance
    {
        VkInstance                  instance            = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT    debugMessenger      = VK_NULL_HANDLE;
        std::vector<const char*>    layers              = {};
        Instance() = default;
        void Destroy() const
        {
            vkb::DestroyDebugUtilsMessenger(instance, debugMessenger, nullptr);
            vkDestroyInstance(instance, nullptr);
        }
    };

    class InstanceBuilder
    {
        private:
            Instance                    m_Result            = {};
            bool                        m_DebugEnabled      = false;
            std::vector<const char*>    m_InstanceLayers;
            std::vector<const char*>    m_IntanceExtensions;
            VkApplicationInfo           m_AppInfo           = {};
            VkInstance                  m_Instance          = VK_NULL_HANDLE;

            void SetupRequiredExtensionsAndLayers();
        public:
            InstanceBuilder() = default;
            ~InstanceBuilder() = default;
            void SetAppName(const std::string& name);
            void SetAppVersion(uint32_t appVersion);
            void SetAppVersion(int minor, int major, int patch);
            void SetEngineName(const std::string& engineName);
            void SetEngineVersion(uint32_t engineVersion);
            void SetEngineVersion(int minor, int major, int patch);
            void RequireAPIVersion(int minor, int major, int patch);
            void RequireAPIVersion(uint32_t version);
            void RequestDebug();

            void Build();
            Instance Get();
    };
}