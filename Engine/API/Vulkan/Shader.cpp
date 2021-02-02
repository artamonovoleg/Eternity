#include <fstream>
#include "Shader.hpp"
#include "Device.hpp"
#include "VkCheck.hpp"
#include "Base.hpp"

namespace Eternity
{
    Shader::Shader(const Device& device, Shader::Type type, const std::string& filename)
        : m_Device(device), m_Type(type)
    {
        CreateShaderModule(ReadFile(filename));
    }

    Shader::~Shader()
    {
        vkDestroyShaderModule(m_Device, m_Module, nullptr);
    }

    std::vector<char> Shader::ReadFile(const std::string& filename) 
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

    void Shader::CreateShaderModule(const std::vector<char>& code)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode    = reinterpret_cast<const uint32_t*>(code.data());

        VkCheck(vkCreateShaderModule(m_Device, &createInfo, nullptr, &m_Module));
    }
} // namespace Eternity
