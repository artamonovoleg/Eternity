#include "CommandBuffer.hpp"
#include "Device.hpp"
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

    void CommandBuffer::Begin()
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        VkCheck(vkBeginCommandBuffer(m_Buffer, &beginInfo));
    }

    void CommandBuffer::Begin(VkCommandBufferBeginInfo* beginInfo)
    {
        VkCheck(vkBeginCommandBuffer(m_Buffer, beginInfo));
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

    void CommandBuffer::End() const
    {
        vkEndCommandBuffer(m_Buffer);
    }
} // namespace Eternity
