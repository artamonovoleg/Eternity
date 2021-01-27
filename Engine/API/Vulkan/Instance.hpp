#pragma once
#include <vulkan/vulkan.h>

namespace Eternity
{
    class Instance
    {
        private:
            VkInstance                  m_Instance = VK_NULL_HANDLE;
            VkDebugUtilsMessengerEXT    m_DebugMessenger = VK_NULL_HANDLE;

            void CreateInstance();
            // Validation layers
            bool        CheckValidationLayersSupport();
            void        PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
            VkResult    CreateDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo);
            void        DestroyDebugUtilsMessengerEXT();
            void        SetupDebugMessenger();
        public:
            Instance();
            ~Instance();

            bool ValidationLayersEnabled();
            
            const std::vector<const char*>& GetLayers();
            std::vector<const char*>        GetExtensions();

            operator VkInstance() { return m_Instance; }
    };
} // namespace Eternity