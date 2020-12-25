//
// Created by artamonovoleg on 25.12.2020.
//

#include <set>
#include <vulkan/vulkan.h>
#include "PhysicalDeviceSelector.hpp"
#include "Base.hpp"

namespace vkb
{
    void PhysicalDeviceSelector::Select()
    {
        uint32_t deviceCount = 0;
        // get gpus count last parameter nullptr
        vkEnumeratePhysicalDevices(m_Instance.instance, &deviceCount, nullptr);
        ET_CORE_ASSERT(deviceCount != 0, "No gpu");
        std::vector<VkPhysicalDevice> devices(deviceCount);
        // now enumerate again and save all gpus to vector
        vkEnumeratePhysicalDevices(m_Instance.instance, &deviceCount, devices.data());
        // TODO: check suitability
        // For now I will take first founded device
        m_PhysicalDevice.physicalDevice = devices[0];
    }

    vkb::PhysicalDevice PhysicalDeviceSelector::Get()
    {
        return m_PhysicalDevice;
    }
}