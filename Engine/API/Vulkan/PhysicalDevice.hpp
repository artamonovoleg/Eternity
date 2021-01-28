#pragma once
#include <optional>
#include <vector>
#include <vulkan/vulkan.h>

namespace Eternity
{
    enum class QueueType
    {
        Graphics,
        Present
    };

    class Instance;
    class Surface;

    class PhysicalDevice
    {
        private:
            struct QueueFamilyIndices 
            {
                std::optional<uint32_t> graphicsFamily;
                std::optional<uint32_t> presentFamily;

                bool isComplete() 
                {
                    return graphicsFamily.has_value() && presentFamily.has_value();
                }
            };

            struct SwapchainSupportDetails 
            {
                VkSurfaceCapabilitiesKHR        capabilities;
                std::vector<VkSurfaceFormatKHR> formats;
                std::vector<VkPresentModeKHR>   presentModes;
            };

            const Surface&      m_Surface;
            VkPhysicalDevice    m_PhysicalDevice;

            uint32_t            m_GraphicsFamily;
            uint32_t            m_PresentFamily;

            bool                    CheckDeviceExtensionSupport(VkPhysicalDevice device);
            QueueFamilyIndices      FindQueueFamilies(VkPhysicalDevice device);
            const SwapchainSupportDetails QuerySwapchainSupport(VkPhysicalDevice device) const;
            bool                    IsDeviceSuitable(VkPhysicalDevice device);
        public:
            PhysicalDevice(const Instance& instance, const Surface& surface);
            ~PhysicalDevice() = default;

            const uint32_t                    GetQueueFamilyIndex(QueueType type) const;
            const SwapchainSupportDetails     GetSwapchainSupportDetails() const;
            const std::vector<const char*>&   GetDeviceExtensions() const;
            
            operator VkPhysicalDevice() { return m_PhysicalDevice; }
    };
} // namespace Eternity
