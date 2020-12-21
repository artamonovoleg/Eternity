//
// Created by artamonovoleg on 20.12.2020.
//

#include "GraphicsContext.hpp"
#include "VkGraphicContext.hpp"
#include "Renderer.hpp"

namespace Eternity
{
    std::shared_ptr<GraphicsContext> GraphicsContext::Create()
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::Vulkan: return std::make_shared<VkGraphicContext>();
            default:                       break;
        }
    }
}
