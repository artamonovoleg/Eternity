//
// Created by artamonovoleg on 20.12.2020.
//

#include "RendererAPI.hpp"
#include "VkRendererAPI.hpp"

namespace Eternity
{
    RendererAPI::API RendererAPI::s_API = RendererAPI::API::Vulkan;

    std::shared_ptr<RendererAPI> RendererAPI::Create()
    {
        switch (s_API)
        {
//            case RendererAPI::API::Vulkan: return std::shared_ptr<VkRendererAPI>();
        }
    }
}