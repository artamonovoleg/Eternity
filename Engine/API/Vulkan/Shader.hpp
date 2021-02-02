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
} // namespace Eternity
