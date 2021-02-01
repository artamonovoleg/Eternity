#include "CommandPool.hpp"
#include "Device.hpp"
#include "PhysicalDevice.hpp"
#include "VkCheck.hpp"
#include "Base.hpp"

namespace Eternity
{
    CommandPool::CommandPool(const Device& device)
        : m_Device(device)
    {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = m_Device.GetPhysicalDevice().GetQueueFamilyIndex(QueueType::Graphics);
        // poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        VkCheck(vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_CommandPool));
        ET_TRACE("Command pool created");
    }

    CommandPool::~CommandPool()
    {
        vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
        ET_TRACE("Command pool destroyed");
    }
} // namespace Eternity
