#pragma once
#include <signal.h>
#include "Logger.hpp"

#define ET_CORE_ASSERT(x, ...) if (!(x)) { Eternity::Logger::Error(__VA_ARGS__); raise(SIGTRAP); }