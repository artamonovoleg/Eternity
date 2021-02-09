#pragma once
#include <vulkan/vulkan.h>

namespace Eternity
{
    class Device;

    class Image
    {
        protected:
            const Device&   m_Device;

            VkImage                 m_Image;
            VkImageView             m_ImageView;
            VkDeviceMemory          m_Memory;
            VkExtent3D              m_Extent;
            VkFormat                m_Format;
            uint32_t                m_MipLevels = 1;

            void CreateImage(const VkExtent3D& extent, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
            void CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
        public:
            Image(const Device& device, const VkExtent3D& extent, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageAspectFlags aspectFlags);
            ~Image();

            VkFormat          GetFormat() { return m_Format; }
            const VkDeviceMemory&   GetImageMemory() const { return m_Memory; };

            const VkImageView GetImageView() const { return m_ImageView; }
            operator VkImage() { return m_Image; }
            operator VkImage() const { return m_Image; }
    };
} // namespace Eternity
