#include "RenderPass.hpp"
#include "VkCheck.hpp"
#include "Device.hpp"
#include "Base.hpp"

namespace Eternity
{
    RenderPass::RenderPass(const Device& device, const std::vector<Attachment>& colorAttachments, Attachment& depthAttachment)
        : m_Device(device)
    {
        std::vector<VkAttachmentDescription> attachments(colorAttachments.begin(), colorAttachments.end());
        attachments.push_back(depthAttachment);

        std::vector<VkAttachmentReference> colorRefs;
        colorRefs.reserve(colorAttachments.size());

        for (const auto& colorAttachment : colorAttachments)
        {
            VkAttachmentReference   colorRef{};
            colorRef.attachment = colorAttachment.GetBinding();
            colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorRefs.emplace_back(colorRef);
        }

        VkAttachmentReference   depthRef{};
        depthRef.attachment = depthAttachment.GetBinding();
        depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = static_cast<uint32_t>(colorRefs.size());
        subpass.pColorAttachments = colorRefs.data();
        subpass.pDepthStencilAttachment = &depthRef;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        VkCheck(vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_RenderPass));
        ET_TRACE("RenderPass created");
    }

    RenderPass::~RenderPass()
    {
        vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);
        ET_TRACE("RenderPass destroyed");
    }
} // namespace Eternity
