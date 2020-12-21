//
// Created by artamonovoleg on 20.12.2020.
//

#pragma once
#include <memory>

namespace Eternity
{
    class GraphicsContext
    {
        private:
        public:
            GraphicsContext() = default;
            virtual ~GraphicsContext() = default;

            static std::shared_ptr<GraphicsContext> Create();
    };
}