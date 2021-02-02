#include "CommandPool.hpp"
#include "Device.hpp"
#include "PhysicalDevice.hpp"
#include "CommandBuffer.hpp"
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

    CommandBuffer  CommandPool::BeginSingleTimeCommands()
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        CommandBuffer buffer(m_Device, *this);
        buffer.Begin(&beginInfo);

        return buffer;
    }

    void CommandPool::EndSingleTimeCommands(const CommandBuffer& buffer)
    {
        buffer.End();

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        const VkCommandBuffer& cmdBuff = buffer;
        submitInfo.pCommandBuffers = &cmdBuff;

        vkQueueSubmit(m_Device.GetQueue(QueueType::Graphics), 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_Device.GetQueue(QueueType::Graphics));

        // vkFreeCommandBuffers(*m_Device, *m_CommandPool, 1, &commandBuffer);
    }
} // namespace Eternity
