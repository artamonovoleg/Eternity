//
// Created by artamonovoleg on 26.12.2020.
//

#pragma once
#include <vulkan/vulkan.h>
#include "InstanceBuilder.hpp"
#include "PhysicalDeviceSelector.hpp"

namespace vkb
{
    enum class QueueType
    {
        Graphics,
        Presentation
    };

    struct Device
    {
        VkDevice        device    = VK_NULL_HANDLE;
        uint32_t        GetQueueFamilyIndex(const vkb::PhysicalDevice& physicalDevice, QueueType type);
        VkQueue         GetQueue(const vkb::PhysicalDevice& physicalDevice, vkb::QueueType type);
        void            Destroy() const;
    };

    class DeviceBuilder
    {
        private:
            friend class Device;
            const vkb::Instance&        m_Instance;
            const vkb::PhysicalDevice&  m_PhysicalDevice;
            vkb::Device                 m_Device = {};
            VkSurfaceKHR                m_Surface   = VK_NULL_HANDLE;
        public:
            DeviceBuilder(const vkb::Instance& instance, const vkb::PhysicalDevice& physicalDevice)
                : m_Instance(instance), m_PhysicalDevice(physicalDevice) {}

            void SetSurface(VkSurfaceKHR surface);
            void Build();
            [[nodiscard]] Device Get() const;
    };
}