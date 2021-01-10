#include "Logger.hpp"

#define ET_CORE_INFO(...)   ::Eternity::Logger::Info(__VA_ARGS__)
#define ET_CORE_TRACE(...)  ::Eternity::Logger::Trace(__VA_ARGS__)
#define ET_CORE_WARN(...)   ::Eternity::Logger::Warn(__VA_ARGS__)
#define ET_CORE_ERROR(...)  ::Eternity::Logger::Error(__VA_ARGS__)