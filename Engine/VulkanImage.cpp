#include "VulkanHelper.hpp"

namespace vkh
{
    VkImageCreateInfo                               ImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent)
    {
        VkImageCreateInfo imageCI
        {
            .sType          = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,

            .imageType      = VK_IMAGE_TYPE_2D,

            .format         = format,
            .extent         = extent,

            .mipLevels      = 1,
            .arrayLayers    = 1,
            .samples        = VK_SAMPLE_COUNT_1_BIT,
            .tiling         = VK_IMAGE_TILING_OPTIMAL,
            .usage          = usageFlags
        };

        return imageCI;
    }

    VkImageViewCreateInfo                           ImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags)
    {
        //build a image-view for the depth image to use for rendering
        VkImageViewCreateInfo imageViewCI
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,

            .image = image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = format
        };

        imageViewCI.subresourceRange.baseMipLevel = 0;
        imageViewCI.subresourceRange.levelCount = 1;
        imageViewCI.subresourceRange.baseArrayLayer = 0;
        imageViewCI.subresourceRange.layerCount = 1;
        imageViewCI.subresourceRange.aspectMask = aspectFlags;

        return imageViewCI;
    }

    VkImageCreateInfo ImageCreateInfo(const VkExtent3D& extent, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage)
    {
        VkImageCreateInfo imageCI
        {
            .sType          = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType      = VK_IMAGE_TYPE_2D,
            .format         = format,
            .mipLevels      = 1,
            .arrayLayers    = 1,
            .samples        = VK_SAMPLE_COUNT_1_BIT,
            .tiling         = tiling,
            .usage          = usage,
            .sharingMode    = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
        };
        imageCI.extent.width   = extent.width;
        imageCI.extent.height  = extent.height;
        imageCI.extent.depth   = extent.depth;

        return imageCI;
    }

    VkImage CreateImage(VkDevice& device, VkPhysicalDevice& physicalDevice, VkImageCreateInfo& imageCI, VkMemoryPropertyFlags properties, VkDeviceMemory& imageMemory)
    {
        VkImage image = VK_NULL_HANDLE;
        auto res = vkCreateImage(device, &imageCI, nullptr, &image);
        vkh::Check(res, "Image create failed");

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device, image, &memRequirements);

        VkMemoryAllocateInfo allocCI{};
        allocCI.sType             = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocCI.allocationSize    = memRequirements.size;
        allocCI.memoryTypeIndex   = FindMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

        res = vkAllocateMemory(device, &allocCI, nullptr, &imageMemory);
        vkh::Check(res, "Allocate memory failed");

        vkBindImageMemory(device, image, imageMemory, 0);

        return image;
    }

    void CreateImage(VkDevice& device, 
                    VkPhysicalDevice& physicalDevice, 
                    uint32_t width, uint32_t height, 
                    VkFormat format, 
                    VkImageTiling tiling, 
                    VkImageUsageFlags usage, 
                    VkMemoryPropertyFlags properties, 
                    VkImage& image, 
                    VkDeviceMemory& imageMemory)
    {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        auto res = vkCreateImage(device, &imageInfo, nullptr, &image);
        vkh::Check(res, "Image create failed");

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

        res = vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory);
        vkh::Check(res, "Allocate memory failed");

        vkBindImageMemory(device, image, imageMemory, 0);
    }

    VkPipelineDepthStencilStateCreateInfo           DepthStencilCreateInfo(VkBool32 bDepthTest, VkBool32 bDepthWrite, VkCompareOp compareOp)
    {
        VkPipelineDepthStencilStateCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        info.depthTestEnable = bDepthTest;
        info.depthWriteEnable = bDepthWrite;
        info.depthCompareOp = (bDepthTest == VK_TRUE) ? compareOp : VK_COMPARE_OP_ALWAYS;
        info.depthBoundsTestEnable = VK_FALSE;
        info.minDepthBounds = 0.0f; // Optional
        info.maxDepthBounds = 1.0f; // Optional
        info.stencilTestEnable = VK_FALSE;
        info.front = {};
        info.back  = {};
        return info;
    }
}