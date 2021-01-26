#pragma once

#include "Logger.hpp"

#ifdef ET_DEBUG
#define ET_TRACE(...)           ::Eternity::Logger::Trace(__VA_ARGS__)
#else
#define ET_TRACE(...)           
#endif
#define ET_INFO(...)            ::Eternity::Logger::Info(__VA_ARGS__) 
#define ET_WARN(...)            ::Eternity::Logger::Warn(__VA_ARGS__)
#define ET_ERROR(...)           ::Eternity::Logger::Error(__VA_ARGS__)