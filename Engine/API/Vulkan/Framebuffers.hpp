#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace Eternity
{
    class Swapchain;
    class RenderPass;
    class DepthImage;

    class Framebuffers
    {
        private:
            const Swapchain&            m_Swapchain;

            std::vector<VkFramebuffer>  m_Buffers;
        public:
            Framebuffers(const Swapchain& swapchain, const RenderPass& renderPass, const DepthImage& depthImage);
            ~Framebuffers();

            size_t GetBuffersCount() { return m_Buffers.size(); }
            
            const std::vector<VkFramebuffer>& GetBuffers() const { return m_Buffers; }
            std::vector<VkFramebuffer>& GetBuffers() { return m_Buffers; }
    };
} // namespace Eternity
