
#pragma once

//Platform detection
#ifdef _WIN32
	#ifdef _WIN64
		#define STRAFE_PLATFORM_WINDOWS
	#else
		#error "x86 Builds are not supported for ragdoll Engine."
	#endif
#elif defined(__APPLE__) || defined (__MACH__)
	#include <TargetConditionals.h>
	#if TARGET_IPHONE_EMULATOR == 1
		#error "IOS emulator is not supported."
	#elif TARGET_OS_PHONE == 1
		#define STRAFE_PLATFORM_IOS
		#error "IOS is not supported."
	#elif TARGET_OS_MAC == 1
		#define STRAFE_PLATFORM_IOS
		#error "MacOS is not supported."
	#else
		#error "Unknown Apple Platform."
	#endif
#elif defined(__ANDROID__) //check android before linux as android has linux kernel
	#define STRAFE_PLATFORM_ANDROID
	#error "Android is not supported."
#elif define(__LINUX__)
	#define STRAFE_PLATFORM_LINUX
	#error "Linux is not supported."
#else
	#error "Unknown Platform."
#endif

#ifdef STRAFE_DEBUG
	#define STRAFE_ENABLE_ASSERTS
	#define _CRTDBG_MAP_ALLOC
	#include <cstdlib>
	#include <crtdbg.h>
#endif

// Assert macros
#ifdef STRAFE_ENABLE_ASSERTS
	#define RD_ASSERT(x, ...) do { if(x) { RD_CORE_FATAL("Assertion failed!"); RD_CORE_ERROR(__VA_ARGS__); __debugbreak(); } } while (0)
	#define RD_CRITICAL_ASSERT(x, ...) RD_ASSERT(x, __VA_ARGS__)
#else
	#define RD_ASSERT(x, ...) do { if(x) { RD_CORE_FATAL("Assertion failed!"); RD_CORE_ERROR(__VA_ARGS__); } } while (0)
	#define RD_CRITICAL_ASSERT(x, ...) RD_ASSERT(x, __VA_ARGS__) if(!x) { RD_CORE_FATAL("Fatal error occured, please consult the logs") exit(EXIT_FAILURE); }
#endif

// Bit macro helper
#define BIT(x) (1 << x)

// Stringify macro
#define STRINGIFY(x) #x

// Function pointer binding with std::functions
#define RD_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)
namespace strafe
{
	class Event;
}
using EventCallbackFn = std::function<void(strafe::Event&)>;

#define RD_LOG_EVENT false
#define RD_LOG_INPUT false
#define RD_OPENGL_DEBUG_LEVEL 1 //0 for errors, //1 for medium, //2 for low, //3 for notifications

//ignore warnings
#pragma warning(push)
#pragma warning(disable : 4201) // Disable warning C4201: nonstandard extension used: nameless struct/union