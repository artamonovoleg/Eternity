#pragma once

#include <vulkan/vulkan.h>

namespace Eternity
{
    class Device;
    class CommandBuffer;

    class CommandPool
    {
        private:
            const Device&   m_Device;

            VkCommandPool   m_CommandPool;
        public:
            CommandPool(const Device& device);
            ~CommandPool();

            CommandBuffer  BeginSingleTimeCommands();
            void    EndSingleTimeCommands(const CommandBuffer& buffer);

            operator VkCommandPool() { return m_CommandPool; }
            operator VkCommandPool() const { return m_CommandPool; }
    };
} // namespace Eternity
