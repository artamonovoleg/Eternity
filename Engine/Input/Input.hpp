#pragma once
#include "KeyCodes.hpp"

enum class PressState;

namespace Eternity
{
    class Input
    {
        private:
            static bool Get(uint16_t code, PressState* buff);
            static bool GetDown(uint16_t code, PressState* buff);
            static bool GetUp(uint16_t code, PressState* buff);
        public:
            static void Init();

            static bool GetKey(uint16_t key);
            static bool GetKeyDown(uint16_t key);
            static bool GetKeyUp(uint16_t key);
    };
} // namespace Eternity