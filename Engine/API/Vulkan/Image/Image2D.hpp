#pragma once

#include <string>
#include "Image.hpp"
#include "Buffer.hpp"

namespace Eternity
{
    class CommandPool;
    class Image2D : public Image
    {
        private:
            const Device&       m_Device;
            const CommandPool&  m_CommandPool;

            VkExtent3D      m_Extent;

            stbi_uc*        m_Pixels;
            VkDeviceSize    m_ImageSize;

            VkExtent3D  LoadImage(const std::string& filename);
            void        TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
            void        CopyBufferToImage(VkBuffer buffer);
        public:
            Image2D(const CommandPool& commandPool, const std::string& filename);
    };
} // namespace Eternity
