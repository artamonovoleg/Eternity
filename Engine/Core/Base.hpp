//
// Created by artamonovoleg on 19.12.2020.
//

#pragma once

#if ET_DEBUG
#if defined(ET_PLATFORM_WINDOWS)
		#define ET_DEBUGBREAK() __debugbreak()
	#elif defined(ET_PLATFORM_LINUX)
		#include <signal.h>
		#define ET_DEBUGBREAK() raise(SIGTRAP)
	#else
		#error "Platform doesn't support debug break yet!"
	#endif
	#define ET_ENABLE_ASSERTS
#else
#define ET_DEBUGBREAK()
#endif

#define ET_EXPAND_MACRO(X) X
#define ET_STRINGIFY_MACRO(X) #X

#include "Log.hpp"
#include "Assert.hpp"