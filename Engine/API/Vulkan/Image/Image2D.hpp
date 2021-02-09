#pragma once

#include <string>
#include "Image.hpp"
#include "Buffer.hpp"

namespace Eternity
{
    class CommandPool;
    class WriteDescriptorSet;

    class Image2D : public Image
    {
        private:
            const Device&       m_Device;
            const CommandPool&  m_CommandPool;

            VkExtent3D      m_Extent;

            stbi_uc*        m_Pixels;
            VkDeviceSize    m_ImageSize;
            VkSampler       m_Sampler;
            
            VkExtent3D  LoadImage(const std::string& filename);
            void        TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
            void        CopyBufferToImage(VkBuffer buffer);
            void        CreateSampler();
            void        GenerateMipmaps(VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
        public:
            Image2D(const CommandPool& commandPool, const std::string& filename);
            ~Image2D();

            const VkSampler&    GetSampler() const { return m_Sampler; };
            WriteDescriptorSet  GetWriteDescriptorSet(uint32_t binding, uint32_t count);

            static VkDescriptorSetLayoutBinding GetDescriptorSetLayout(uint32_t binding, uint32_t count);
    };
} // namespace Eternity
