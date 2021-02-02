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

            void                    WaitIdle();
            VkImageView             CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) const;

            const PhysicalDevice&   GetPhysicalDevice() const { return m_PhysicalDevice; }
            VkQueue                 GetQueue(QueueType type) const;

            operator VkDevice() { return m_Device; }
            operator VkDevice() const { return m_Device; }
    };
}