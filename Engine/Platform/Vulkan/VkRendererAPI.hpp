////
//// Created by artamonovoleg on 20.12.2020.
////
//
//#pragma once
//#include <vector>
//#include <vulkan/vulkan.h>
//#include "RendererAPI.hpp"
//
//namespace Eternity
//{
//    class VkRendererAPI : public RendererAPI
//    {
//        private:
//            VkInstance                  m_Instance  = VK_NULL_HANDLE;
//            VkPhysicalDevice            m_GPU       = VK_NULL_HANDLE;
//            VkPhysicalDeviceProperties  m_GPUProperties{};
//            VkDevice                    m_Device    = VK_NULL_HANDLE;
//            VkQueue                     m_Queue     = VK_NULL_HANDLE;
//            uint32_t                    m_GraphicsFamilyIndex = 0;
//
//            std::vector<const char*>    m_InstanceLayers;
//            std::vector<const char*>    m_InstanceExtensions;
//
//            std::vector<const char*>    m_DeviceLayers;
//            std::vector<const char*>    m_DeviceExtensions;
//
//            // TODO: Move it from renderer. And other debug
//            VkDebugReportCallbackEXT    m_DebugReport = VK_NULL_HANDLE;
//            VkDebugReportCallbackCreateInfoEXT m_DebugReportCallbackCreateInfo{};
//
//            void InitInstance();
//            void DeinitInstance();
//
//            void InitDevice();
//            void DeinitDevice();
//
//            void SetupLayersAndExtensions();
//
//            void SetupDebug();
//            void InitDebug();
//            void DeinitDebug();
//        public:
//            VkRendererAPI();
//            ~VkRendererAPI();
//
//            [[nodiscard]] VkInstance                    GetVulkanInstance()                 const;
//            [[nodiscard]] VkPhysicalDevice              GetVulkanPhysicalDevice()           const;
//            [[nodiscard]] VkDevice                      GetVulkanDevice()                   const;
//            [[nodiscard]] VkQueue                       GetVulkanQueue()                    const;
//            [[nodiscard]] uint32_t                      GetVulkanGraphicsFamilyIndex()      const;
//            [[nodiscard]] const VkPhysicalDeviceProperties&   GetVulkanPhysicalDeviceProperties() const;
//    };
//}