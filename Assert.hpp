#pragma once
#include "Base.hpp"
#include "Log.hpp"

#ifdef ET_ENABLE_ASSERTS

// Alteratively we could use the same "default" message for both "WITH_MSG" and "NO_MSG" and
	// provide support for custom formatting by concatenating the formatting string instead of having the format inside the default message
	#define ET_INTERNAL_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { ET##type##ERROR(msg, __VA_ARGS__); ET_DEBUGBREAK(); } }
	#define ET_INTERNAL_ASSERT_WITH_MSG(type, check, ...) ET_INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
	#define ET_INTERNAL_ASSERT_NO_MSG(type, check) ET_INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", ET_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)

	#define ET_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
	#define ET_INTERNAL_ASSERT_GET_MACRO(...) ET_EXPAND_MACRO( ET_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, ET_INTERNAL_ASSERT_WITH_MSG, ET_INTERNAL_ASSERT_NO_MSG) )

	// Currently accepts at least the condition and one additional parameter (the message) being optional
	#define ET_ASSERT(...) ET_EXPAND_MACRO( ET_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )
	#define ET_CORE_ASSERT(...) ET_EXPAND_MACRO( ET_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_CORE_, __VA_ARGS__) )
#else
#define ET_ASSERT(...)
#define ET_CORE_ASSERT(...)
#endif