#pragma once
#include <vector>
#include <vulkan/vulkan.h>

namespace Eternity
{
    class Instance
    {
        private:
            VkInstance                  m_Instance = VK_NULL_HANDLE;
            VkDebugUtilsMessengerEXT    m_DebugMessenger = VK_NULL_HANDLE;

            void        CreateInstance();
            // Validation layers
            bool        CheckValidationLayersSupport();
            void        PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
            VkResult    CreateDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo);
            void        DestroyDebugUtilsMessengerEXT();
            void        SetupDebugMessenger();
        public:
            Instance();
            ~Instance();

            const bool                          ValidationLayersEnabled() const;
            
            const std::vector<const char*>&     GetLayers() const;
            const std::vector<const char*>      GetExtensions() const;

            operator VkInstance() { return m_Instance; }
            operator VkInstance() const { return m_Instance; }
    };
} // namespace Eternity