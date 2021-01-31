#include "Image.hpp"
#include "Device.hpp"
#include "Utils.hpp"
#include "VkCheck.hpp"
#include "Base.hpp"

namespace Eternity
{
    Image::Image(const Device& device, const VkExtent3D& extent, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties)
        : m_Device(device)
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

    Image::~Image()
    {
        vkFreeMemory(m_Device, m_Memory, nullptr);
        ET_TRACE("Free image memory");
        vkDestroyImage(m_Device, m_Image, nullptr);
    }

} // namespace Eternity
