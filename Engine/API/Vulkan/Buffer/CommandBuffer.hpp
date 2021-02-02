#pragma once

#include <functional>
#include <vulkan/vulkan.h>

namespace Eternity
{
    class Device;
    class CommandPool;

    class CommandBuffer
    {
        private:
            const Device&       m_Device;
            const CommandPool&  m_CommandPool;

            VkCommandBuffer     m_Buffer;
        public:
            CommandBuffer(const Device& device, const CommandPool& commandPool);
            ~CommandBuffer();

            void Begin() const;
            void BeginSingleTime() const;
            void BeginRenderPass(const VkRenderPassBeginInfo* beginInfo, const VkSubpassContents& contents);
            void BindPipeline(VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline);
            void EndRenderPass();

            void EndSingleTime() const;
            void End() const;

            operator VkCommandBuffer() { return m_Buffer; }
            operator VkCommandBuffer() const { return m_Buffer; }
    };
} // namespace Eternity
