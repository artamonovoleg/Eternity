#include <array>
#include "DescriptorPool.hpp"
#include "Device.hpp"
#include "Swapchain.hpp"
#include "VkCheck.hpp"
#include "Base.hpp"

namespace Eternity
{
    DescriptorPool::DescriptorPool(const Swapchain& swapchain, const std::vector<DescriptorType>& types)
        : m_Device(swapchain.GetDevice())
    {
        std::vector<VkDescriptorPoolSize> poolSizes(types.size());
        uint32_t descriptorCount = swapchain.GetImageCount();

        for (int i = 0; i < poolSizes.size(); i++)
        {
            poolSizes[i].descriptorCount = descriptorCount;
            switch (types[i])
            {
                case DescriptorType::Uniform:
                    poolSizes[i].type   = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                    break;
                case DescriptorType::ImageSampler:
                    poolSizes[i].type   = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    break;
            }
        }

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType          = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount  = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes     = poolSizes.data();
        poolInfo.maxSets        = descriptorCount;

        VkCheck(vkCreateDescriptorPool(m_Device, &poolInfo, nullptr, &m_DescriptorPool));
        ET_TRACE("Descriptor pool created");
    }

    DescriptorPool::~DescriptorPool()
    {
        vkDestroyDescriptorPool(m_Device, m_DescriptorPool, nullptr);
        ET_TRACE("Descriptor pool destroyed");
    }
} // namespace Eternity
