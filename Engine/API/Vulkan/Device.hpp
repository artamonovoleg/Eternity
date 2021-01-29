#pragma once
#include <vulkan/vulkan.h>

namespace Eternity
{
    class Instance;
    class PhysicalDevice;
    enum class QueueType;

    class Device
    {
        private:
            const PhysicalDevice& m_PhysicalDevice;

            VkDevice    m_Device;
            VkQueue     m_GraphicsQueue;
            VkQueue     m_PresentQueue;
        public:
            Device(const Instance& instance, const PhysicalDevice& physicalDevice);
            ~Device();

            void        WaitIdle();
            
            VkQueue     GetQueue(QueueType type);

            operator VkDevice() { return m_Device; }
    };
}