//
// Created by artamonovoleg on 25.12.2020.
//

#pragma once
#include "InstanceBuilder.hpp"

namespace vkb
{
    struct PhysicalDevice
    {
        VkPhysicalDevice        device;
        VkSurfaceKHR            surface;
    };

    class PhysicalDeviceSelector
    {
        private:
            const vkb::Instance&            m_Instance          = {};
            vkb::PhysicalDevice             m_PhysicalDevice    = {};
        public:
            explicit PhysicalDeviceSelector(const vkb::Instance& instance)
                : m_Instance(instance) {}
            ~PhysicalDeviceSelector() = default;
            void SetSurface(VkSurfaceKHR surface);
            void                    Select();
            [[nodiscard]]
            vkb::PhysicalDevice     Get() const;
    };
}