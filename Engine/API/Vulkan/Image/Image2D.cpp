#include <cstring>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "Image2D.hpp"
#include "Device.hpp"

namespace Eternity
{
    VkExtent3D Image2D::LoadImage(const std::string& filename)
    {
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * 4;

        if (!pixels) 
            throw std::runtime_error("failed to load texture image!");

        Buffer stageBuff(m_Device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        void* data;
        stageBuff.MapMemory(&data);
            std::memcpy(data, pixels, static_cast<size_t>(imageSize));
        stageBuff.UnmapMemory();

        stbi_image_free(pixels);
    }

    Image2D::Image2D(const Device& device, const std::string& filename)
        : Image(m_Device, LoadImage(filename), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_COLOR_BIT);
    {
        
    };
} // namespace Eternity
