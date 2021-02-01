#include <array>
#include "Framebuffers.hpp"
#include "Device.hpp"
#include "Swapchain.hpp"
#include "RenderPass.hpp"
#include "DepthImage.hpp"
#include "VkCheck.hpp"
#include "Base.hpp"

namespace Eternity
{
    Framebuffers::Framebuffers(const Swapchain& swapchain, const RenderPass& renderPass, const DepthImage& depthImage)
        : m_Swapchain(swapchain)
    {
        m_Buffers.resize(m_Swapchain.GetImageCount());

        for (size_t i = 0; i < m_Swapchain.GetImageCount(); i++) 
        {
            std::array<VkImageView, 2> attachments = {
                m_Swapchain.GetImageViews()[i],
                depthImage.GetImageView()
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass      = renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments    = attachments.data();
            framebufferInfo.width           = m_Swapchain.GetExtent().width;
            framebufferInfo.height          = m_Swapchain.GetExtent().height;
            framebufferInfo.layers          = 1;

            VkCheck(vkCreateFramebuffer(m_Swapchain.GetDevice(), &framebufferInfo, nullptr, &m_Buffers[i]));
        }
        
        ET_TRACE("Framebuffers created");
    }

    Framebuffers::~Framebuffers()
    {
        for (auto buffer : m_Buffers)
            vkDestroyFramebuffer(m_Swapchain.GetDevice(), buffer, nullptr);
        ET_TRACE("Framebuffers destroyed");
    }
} // namespace Eternity
