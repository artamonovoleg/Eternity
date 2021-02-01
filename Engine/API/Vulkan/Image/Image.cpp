#include "Image.hpp"
#include "Device.hpp"
#include "Utils.hpp"
#include "VkCheck.hpp"
#include "Base.hpp"

namespace Eternity
{
    Image::Image(const Device& device, const VkExtent3D& extent, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImageAspectFlags aspectFlags)
        : m_Device(device), m_Format(format)
    {
        CreateImage(extent, format, tiling, usage, properties);
        CreateImageView(m_Image, format, aspectFlags);
    }

    Image::~Image()
    {
        vkDestroyImageView(m_Device, m_ImageView, nullptr);
        ET_TRACE("Image view destroyed");
        vkFreeMemory(m_Device, m_Memory, nullptr);
        ET_TRACE("Free image memory");
        vkDestroyImage(m_Device, m_Image, nullptr);
        ET_TRACE("Image destroyed");
    }

    void Image::CreateImage(const VkExtent3D& extent, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties)
    {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType     = VK_IMAGE_TYPE_2D;
        imageInfo.extent        = extent;
        imageInfo.mipLevels     = 1;
        imageInfo.arrayLayers   = 1;
        imageInfo.format        = format;
        imageInfo.tiling        = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage         = usage;
        imageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;

        VkCheck(vkCreateImage(m_Device, &imageInfo, nullptr, &m_Image));
        ET_TRACE("Image created");

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(m_Device, m_Image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(m_Device.GetPhysicalDevice(), memRequirements.memoryTypeBits, properties);

        VkCheck(vkAllocateMemory(m_Device, &allocInfo, nullptr, &m_Memory));
        ET_TRACE("Allocate image memory");

        vkBindImageMemory(m_Device, m_Image, m_Memory, 0);
    }

    void Image::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType                              = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image                              = image;
        viewInfo.viewType                           = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format                             = format;
        viewInfo.subresourceRange.aspectMask        = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel      = 0;
        viewInfo.subresourceRange.levelCount        = 1;
        viewInfo.subresourceRange.baseArrayLayer    = 0;
        viewInfo.subresourceRange.layerCount        = 1;

        VkCheck(vkCreateImageView(m_Device, &viewInfo, nullptr, &m_ImageView));
        ET_TRACE("ImageView created");
    }

} // namespace Eternity
