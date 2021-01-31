#pragma once
#include <vulkan/vulkan.h>

namespace Eternity
{
    class Device;

    class Image
    {
        private:
            const Device&   m_Device;

            VkImage         m_Image;
            VkDeviceMemory  m_Memory;
        public:
            Image(const Device& device, const VkExtent3D& extent, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
            ~Image();

            VkDeviceMemory  GetImageMemory() { return m_Memory; };
            operator VkImage() { return m_Image; }
            operator VkImage() const { return m_Image; }
    };
} // namespace Eternity
