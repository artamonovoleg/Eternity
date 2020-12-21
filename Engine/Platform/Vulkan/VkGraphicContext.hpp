//
// Created by artamonovoleg on 21.12.2020.
//

#pragma once
#include "GraphicsContext.hpp"

namespace Eternity
{
    class VkGraphicContext : public GraphicsContext
    {
        private:
        public:
            VkGraphicContext()              = default;
            ~VkGraphicContext() override    = default;
    };
}