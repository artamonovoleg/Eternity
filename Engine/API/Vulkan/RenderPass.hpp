#pragma once

#include <optional>
#include <vector>
#include <vulkan/vulkan.h>

namespace Eternity
{
    class Attachment
    {
        public:
            enum class Type
            {
                Color, Depth
            };
        private:
            VkAttachmentDescription m_Attachment{};
            uint32_t m_Binding;
            Type     m_Type;
        public:
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
                        break;
                    case Type::Depth:
                        m_Attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                        m_Attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                        break;
                    default:
                        break;
                }
            }

            const uint32_t                  GetBinding() const { return m_Binding; }
            const Type                      GetType() const { return m_Type; }
            operator VkAttachmentDescription() { return m_Attachment; }
            operator VkAttachmentDescription() const { return m_Attachment; }
    };

    class Device;

    class RenderPass
    {
        private:
            const Device&   m_Device;
            VkRenderPass    m_RenderPass;
        public:
            RenderPass(const Device& device)
                : m_Device(device)
            {
            }

            ~RenderPass();

            operator VkRenderPass() { return m_RenderPass; }
    };
} // namespace Eternity
