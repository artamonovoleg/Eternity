#pragma once

#include <string>
#include "Image.hpp"
#include "Buffer.hpp"

namespace Eternity
{
    class Image2D : public Image
    {
        private:
            const Device&   m_Device;
            Buffer          m_StageBuffer;
            VkExtent3D      m_Extent;

            VkExtent3D LoadImage(const std::string& filename);
        public:
            Image2D(const Device& device, const std::string& filename);
    };
} // namespace Eternity
