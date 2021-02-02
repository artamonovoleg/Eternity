#include "CommandBuffer.hpp"
#include "Device.hpp"
#include "PhysicalDevice.hpp"
#include "CommandPool.hpp"
#include "VkCheck.hpp"
#include "Base.hpp"

namespace Eternity
{
    CommandBuffer::CommandBuffer(const Device& device, const CommandPool& commandPool)
        : m_Device(device), m_CommandPool(commandPool)
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType                 = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool           = m_CommandPool;
        allocInfo.level                 = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount    = 1;

        VkCheck(vkAllocateCommandBuffers(m_Device, &allocInfo, &m_Buffer));
        ET_TRACE("Allocate command buffer");
    }

    CommandBuffer::~CommandBuffer()
    {
        vkFreeCommandBuffers(m_Device, m_CommandPool, 1, &m_Buffer);
        ET_TRACE("Free command buffer");
    }

    void CommandBuffer::Begin() const
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        VkCheck(vkBeginCommandBuffer(m_Buffer, &beginInfo));
    }

    void CommandBuffer::BeginSingleTime() const
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        VkCheck(vkBeginCommandBuffer(m_Buffer, &beginInfo));
    }

    void CommandBuffer::BeginRenderPass(const VkRenderPassBeginInfo* beginInfo, const VkSubpassContents& contents)
    {
        vkCmdBeginRenderPass(m_Buffer, beginInfo, contents);
    }

    void CommandBuffer::BindPipeline(VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline)
    {
        vkCmdBindPipeline(m_Buffer, pipelineBindPoint, pipeline);
    }

    void CommandBuffer::EndRenderPass()
    {
        vkCmdEndRenderPass(m_Buffer);
    }

    void CommandBuffer::EndSingleTime() const
    {
        vkEndCommandBuffer(m_Buffer);
                
        VkSubmitInfo submitInfo{};
        submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount   = 1;
        submitInfo.pCommandBuffers      = &m_Buffer;

        vkQueueSubmit(m_Device.GetQueue(QueueType::Graphics), 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_Device.GetQueue(QueueType::Graphics));
    }

    void CommandBuffer::End() const
    {
        vkEndCommandBuffer(m_Buffer);
    }
} // namespace Eternity
