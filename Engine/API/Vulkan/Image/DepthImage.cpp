#include "DepthImage.hpp"
#include "Device.hpp"
#include "Utils.hpp"
#include "Base.hpp"

namespace Eternity
{
    DepthImage::DepthImage(const Device& device, const VkExtent2D& extent)
        : Image(    
                device,                                         // class Device
                { extent.width, extent.height, 1 },             // extent (VkExtent3D)
                FindDepthFormat(device.GetPhysicalDevice()),    // format
                VK_IMAGE_TILING_OPTIMAL,                        // tiling
                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,    // usage
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,            // properties
                VK_IMAGE_ASPECT_DEPTH_BIT
                ){}
} // namespace Eternity
