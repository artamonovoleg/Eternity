#include <array>
#include "Descriptors.hpp"
#include "Device.hpp"
#include "Swapchain.hpp"
#include "VkCheck.hpp"
#include "Base.hpp"

namespace Eternity
{
    /// Descriptor set layout
    
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

    /// Descriptor pool

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
