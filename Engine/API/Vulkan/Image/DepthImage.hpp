#pragma once
#include "Image.hpp"

namespace Eternity
{
    class DepthImage : public Image
    {
        private:
        public:
            DepthImage(const Device& device, const VkExtent2D& extent);
            ~DepthImage() = default;
    };    
} // namespace Eternity
