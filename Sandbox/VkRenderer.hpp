//
// Created by artamonovoleg on 21.12.2020.
//

#pragma once
#include <vector>
#include <vulkan/vulkan.h>

namespace Eternity
{
    class VkRenderer
    {
        private:
            VkInstance                  m_Instance  = VK_NULL_HANDLE;
            VkPhysicalDevice            m_GPU       = VK_NULL_HANDLE;
            VkPhysicalDeviceProperties  m_GPUProperties{};
            VkDevice                    m_Device    = VK_NULL_HANDLE;
            VkQueue                     m_Queue     = VK_NULL_HANDLE;
            uint32_t                    m_GraphicsFamilyIndex = 0;

            std::vector<const char*>    m_InstanceLayers;
            std::vector<const char*>    m_InstanceExtensions;

            std::vector<const char*>    m_DeviceLayers;
            std::vector<const char*>    m_DeviceExtensions;

            void InitInstance();
            void DeinitInstance();

            void InitDevice();
            void DeinitDevice();

            void SetupLayersAndExtensions();

        public:
            VkRenderer();
            ~VkRenderer();

            [[nodiscard]] VkInstance                    GetVulkanInstance()                 const;
            [[nodiscard]] VkPhysicalDevice              GetVulkanPhysicalDevice()           const;
            [[nodiscard]] VkDevice                      GetVulkanDevice()                   const;
            [[nodiscard]] VkQueue                       GetVulkanQueue()                    const;
            [[nodiscard]] uint32_t                      GetVulkanGraphicsFamilyIndex()      const;
            [[nodiscard]] const VkPhysicalDeviceProperties&   GetVulkanPhysicalDeviceProperties() const;
    };
}