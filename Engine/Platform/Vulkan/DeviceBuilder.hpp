//
// Created by artamonovoleg on 26.12.2020.
//

#pragma once
#include <vulkan/vulkan.h>
#include "InstanceBuilder.hpp"
#include "PhysicalDeviceSelector.hpp"

namespace vkb
{
    struct Device
    {
        VkDevice        device    = VK_NULL_HANDLE;
        uint32_t        graphicsQueueFamilyIndex;
        uint32_t        presentQueueFamilyIndex;
        void Destroy() const
        {
            vkDestroyDevice(device, nullptr);
        }
    };

    class DeviceBuilder
    {
        private:
            const vkb::Instance&        m_Instance;
            const vkb::PhysicalDevice&  m_PhysicalDevice;
            vkb::Device                 m_Device = {};
            VkSurfaceKHR                m_Surface   = VK_NULL_HANDLE;
            uint32_t                    m_GraphicsQueueFamily;
            uint32_t                    m_PresentQueueFamily;
            void FindQueueFamilies();
        public:
            DeviceBuilder(const vkb::Instance& instance, const vkb::PhysicalDevice& physicalDevice)
                : m_Instance(instance), m_PhysicalDevice(physicalDevice) {}

            void SetSurface(VkSurfaceKHR surface);
            void Build();
            [[nodiscard]] Device Get() const;
    };
}