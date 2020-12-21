//
// Created by artamonovoleg on 20.12.2020.
//

#pragma once
#include "RendererAPI.hpp"

namespace Eternity
{
    class Renderer
    {
        private:
            static std::shared_ptr<RendererAPI> s_RendererAPI;
        public:
            static RendererAPI::API GetAPI() { return s_RendererAPI->GetAPI(); }
    };
}
