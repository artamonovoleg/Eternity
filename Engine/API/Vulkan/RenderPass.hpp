#pragma once

#include <optional>
#include <vector>
#include <vulkan/vulkan.h>

namespace Eternity
{
    class Attachment
    {
        private:
            VkAttachmentDescription m_Attachment{};
            VkAttachmentReference   m_Reference{};
            uint32_t m_Binding;
        public:
            enum class Type
            {
                Color, Depth
            };

            Attachment(Attachment::Type type, uint32_t binding, VkFormat format)
                : m_Binding(binding)
            {
                m_Attachment.format = format;
                m_Attachment.samples = VK_SAMPLE_COUNT_1_BIT;
                m_Attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                m_Attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                m_Attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                m_Attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                
                switch (type)
                {
                    case Type::Color:
                        m_Attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                        m_Attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

                        m_Reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                        break;
                    case Type::Depth:
                        m_Attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                        m_Attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

                        m_Reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                        break;
                    default:
                        break;
                }

                m_Reference.attachment = binding;
            }

            const uint32_t                  GetBinding() const { return m_Binding; }
            const VkAttachmentReference&    GetReference() const { return m_Reference; }

            operator VkAttachmentDescription() { return m_Attachment; }
            operator VkAttachmentDescription() const { return m_Attachment; }
    };

    class Device;

    class RenderPass
    {
        private:
            class SubpassDescription
            {
                private:
                    VkSubpassDescription                        m_Description{};
                public:
                    SubpassDescription(const std::vector<VkAttachmentReference>& colorAttachments, std::optional<VkAttachmentReference> depthAttachment)
                    {
                        m_Description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
                        m_Description.colorAttachmentCount = colorAttachments.size();
                        m_Description.pColorAttachments = colorAttachments.data();
                        if (depthAttachment.has_value())
                            m_Description.pDepthStencilAttachment = &depthAttachment.value();
                    }

                    operator VkSubpassDescription() { return m_Description; }
            };

            const Device&   m_Device;
            VkRenderPass    m_RenderPass;
        public:
            RenderPass(const Device& device, const std::vector<Attachment>& colorAttachments, std::optional<Attachment> depthAttachment);
            ~RenderPass();

            operator VkRenderPass() { return m_RenderPass; }
    };
} // namespace Eternity
