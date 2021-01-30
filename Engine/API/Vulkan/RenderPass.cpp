#include "RenderPass.hpp"
#include "VkCheck.hpp"
#include "Device.hpp"
#include "Base.hpp"

namespace Eternity
{
    // RenderPass::RenderPass(const Device& device, const std::vector<Attachment>& colorAttachments, std::optional<Attachment> depthAttachment)
    //     : m_Device(device)
    // {

    //     // VkCheck(vkCreateRenderPass(m_Device, &renderPassInfo, nullptr, &m_RenderPass));
    //     ET_TRACE("RenderPass created");
    // }

    // RenderPass::~RenderPass()
    // {
    //     vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);
    //     ET_TRACE("RenderPass destroyed");
    // }
} // namespace Eternity
