#pragma once

#include <memory>

#include "PlatformDetection.hpp"

#ifdef ET_DEBUG
	#if defined(ET_PLATFORM_WINDOWS)
		#define ET_DEBUGBREAK() __debugbreak()
	#elif defined(ET_PLATFORM_LINUX)
		#include <signal.h>
		#define ET_DEBUGBREAK() raise(SIGTRAP)
	#else
		#error "Platform doesn't support debugbreak yet!"
	#endif
	#define ET_ENABLE_ASSERTS
#else
	#define ET_DEBUGBREAK()
#endif

#define ET_EXPAND_MACRO(x) x
#define ET_STRINGIFY_MACRO(x) #x

#include "Log.hpp"
#include "Assert.hpp"