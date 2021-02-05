#include <array>
#include "Descriptors.hpp"
#include "Device.hpp"
#include "Swapchain.hpp"
#include "VkCheck.hpp"
#include "Base.hpp"

namespace Eternity
{
    DescriptorSetLayout::DescriptorSetLayout(const Device& device, const std::vector<VkDescriptorSetLayoutBinding>& bindings)
        : m_Device(device)
    {
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        VkCheck(vkCreateDescriptorSetLayout(m_Device, &layoutInfo, nullptr, &m_Layout));
        ET_TRACE("DescriptorSetLayout created");
    }

    DescriptorSetLayout::~DescriptorSetLayout()
    {
        vkDestroyDescriptorSetLayout(m_Device, m_Layout, nullptr);
        ET_TRACE("DescriptorSetLayout destroyed");
    }
} // namespace Eternity
