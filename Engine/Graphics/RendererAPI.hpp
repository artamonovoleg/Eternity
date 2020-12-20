//
// Created by artamonovoleg on 20.12.2020.
//

#pragma once
#include <memory>

namespace Eternity
{
    class RendererAPI
    {
        public:
            enum class API
            {
                    None,
                    Vulkan
            };
        private:
            static RendererAPI::API s_API;
        public:
            static std::shared_ptr<RendererAPI> Create();
    };
}