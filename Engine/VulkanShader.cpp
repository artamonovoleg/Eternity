#include <fstream>
#include "VulkanHelper.hpp"

namespace vkh
{
    std::vector<char>                               ReadShader(const std::string& filename) 
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open())
            throw std::runtime_error("failed to open file!");

        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();
        return buffer;
    }

    VkShaderModule                                  CreateShaderModule(const VkDevice& device, const std::string& filename)
    {
        VkShaderModule shaderModule = VK_NULL_HANDLE;

        auto shaderCode = ReadShader(filename);
        //create a new shader module, using the buffer we loaded
        VkShaderModuleCreateInfo shaderModuleCI
        {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            //codeSize has to be in bytes, so multply the ints in the buffer by size of int to know the real size of the buffer
            .codeSize = shaderCode.size(),
            .pCode = reinterpret_cast<const uint32_t*>(shaderCode.data())
        };

        //check that the creation goes well.
        vkh::Check(vkCreateShaderModule(device, &shaderModuleCI, nullptr, &shaderModule), "Shader module create failed");
        return shaderModule;
    }
}