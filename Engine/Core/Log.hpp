//
// Created by artamonovoleg on 20.12.2020.
//

#pragma once
#include "Logger.hpp"

#if ET_DEBUG
#define ET_CORE_PERFORMANCE(...)    ::Eternity::Logger::Performance(__VA_ARGS__)
#define ET_CORE_TRACE(...)          ::Eternity::Logger::Trace(__VA_ARGS__)
#define ET_CORE_INFO(...)           ::Eternity::Logger::Info(__VA_ARGS__)
#define ET_CORE_WARN(...)           ::Eternity::Logger::Warn(__VA_ARGS__)
#define ET_CORE_ERROR(...)          ::Eternity::Logger::Error(__VA_ARGS__)
#define ET_CORE_FATAL(...)          ::Eternity::Logger::Error(__VA_ARGS__)
#else
#define ET_CORE_PERFORMANCE(...)
#define ET_CORE_TRACE(...)
#define ET_CORE_INFO(...)
#define ET_CORE_WARN(...)
#define ET_CORE_ERROR(...)
#define ET_CORE_FATAL(...)
#endif
