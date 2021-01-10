#include "VulkanHelper.hpp"

namespace vkh
{
    VkBuffer                                        CreateVertexBuffer(const VkDevice& device, const VkPhysicalDevice& physicalDevice, uint32_t size, VkDeviceMemory& memory)
    {
        VkBuffer vertexBuffer = VK_NULL_HANDLE;
        VkBufferCreateInfo bufferCI
        {
            .sType          = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            //this is the total size, in bytes, of the buffer we are allocating
            .size           = size,
            //this buffer is going to be used as a Vertex Buffer
            .usage          = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            .sharingMode    = VK_SHARING_MODE_EXCLUSIVE
        };

        auto res = vkCreateBuffer(device, &bufferCI, nullptr, &vertexBuffer);
        vkh::Check(res, "Buffer create failed");
        
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, vertexBuffer, &memRequirements);

        VkMemoryAllocateInfo allocCI{};
        allocCI.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocCI.allocationSize  = memRequirements.size;
        allocCI.memoryTypeIndex = vkh::FindMemoryType(physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        res = vkAllocateMemory(device, &allocCI, nullptr, &memory);
        vkh::Check(res, "Memory allocate failed");
        vkBindBufferMemory(device, vertexBuffer, memory, 0);

        return vertexBuffer;
    }
}