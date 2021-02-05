#include "DescriptorSets.hpp"
#include "Device.hpp"
#include "Swapchain.hpp"
#include "Descriptors.hpp"
#include "DescriptorPool.hpp"
#include "VkCheck.hpp"
#include "Base.hpp"

namespace Eternity
{
    DescriptorSets::DescriptorSets(const Swapchain& swapchain, const DescriptorSetLayout& layout, const DescriptorPool& descriptorPool)
        : m_Swapchain(swapchain), m_DescriptorPool(descriptorPool)
    {
        std::vector<VkDescriptorSetLayout> layouts(m_Swapchain.GetImageCount(), layout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType                 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool        = m_DescriptorPool;
        allocInfo.descriptorSetCount    = static_cast<uint32_t>(m_Swapchain.GetImageCount());
        allocInfo.pSetLayouts           = layouts.data();

        m_DescriptorSets.resize(m_Swapchain.GetImageCount());
        VkCheck(vkAllocateDescriptorSets(m_Swapchain.GetDevice(), &allocInfo, m_DescriptorSets.data()));
        ET_TRACE("Allocate descriptor sets");
    }

    void DescriptorSets::UpdateSets(uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount /*= 0*/, const VkCopyDescriptorSet* pDescriptorCopies /*= nullptr*/)
    {
        vkUpdateDescriptorSets(m_Swapchain.GetDevice(), descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
    }

} // namespace Eternity
