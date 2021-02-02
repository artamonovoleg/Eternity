#pragma once

#include <string>
#include <vector>
#include <vulkan/vulkan.h>

namespace Eternity
{   
    class Device;

    class Shader
    {
        public:
            enum class Type
            {
                Vertex, Fragment, Geometry
            };
        private:
            const Type      m_Type;
            const Device&   m_Device;
            
            VkShaderModule  m_Module;

            static std::vector<char>    ReadFile(const std::string& filename);
            void                        CreateShaderModule(const std::vector<char>& code);
        public:
            Shader(const Device& device, Shader::Type type, const std::string& filename);
            ~Shader();

            const Type GetType() const { return m_Type; }
            
            operator VkShaderModule() { return m_Module; }
            operator VkShaderModule() const { return m_Module; }
    };

    // TODO: rewrite to initializer_list or vector
    class ShaderStage
    {
        private:
            std::vector<VkPipelineShaderStageCreateInfo> m_Stages;
        public:
            ShaderStage(const Shader& vertShader, const Shader& fragShader)
            {
                VkPipelineShaderStageCreateInfo info{};
                info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                info.module = vertShader;
                info.pName = "main";
                info.stage = VK_SHADER_STAGE_VERTEX_BIT;
                m_Stages.push_back(info);

                info.module = fragShader;
                info.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
                m_Stages.push_back(info);
            }

            const VkPipelineShaderStageCreateInfo* GetStages() const { return m_Stages.data(); }
    };
} // namespace Eternity
