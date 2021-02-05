#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace Eternity
{
    class Swapchain;
    class DescriptorSetLayout;
    class DescriptorPool;

    class DescriptorSets
    {
        private:
            const Swapchain&        m_Swapchain;
            const DescriptorPool&   m_DescriptorPool;

            std::vector<VkDescriptorSet> m_DescriptorSets;
        public:
            DescriptorSets(const Swapchain& swapchain, const DescriptorSetLayout& layout, const DescriptorPool& descriptorPool);
            ~DescriptorSets() = default;

            void UpdateSets(uint32_t descriptorWriteCount, const VkWriteDescriptorSet* pDescriptorWrites, uint32_t descriptorCopyCount = 0, const VkCopyDescriptorSet* pDescriptorCopies = nullptr);

            const VkDescriptorSet& GetSet(int index) const { return m_DescriptorSets.at(index); }
    };
} // namespace Eternity
