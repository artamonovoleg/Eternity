//
// Created by artamonovoleg on 25.12.2020.
//

#pragma once
#include "InstanceBuilder.hpp"

namespace vkb
{
    struct PhysicalDevice
    {
        VkPhysicalDevice physicalDevice;
    };

    class PhysicalDeviceSelector
    {
        private:
            vkb::Instance           m_Instance          = {};
            vkb::PhysicalDevice     m_PhysicalDevice    = {};
        public:
            explicit PhysicalDeviceSelector(vkb::Instance& instance)
                : m_Instance(instance) {}
            ~PhysicalDeviceSelector() = default;

            void                Select();
            vkb::PhysicalDevice Get();
    };
}