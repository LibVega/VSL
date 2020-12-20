/*
 * Microsoft Public License (Ms-PL) - Copyright (c) 2020 Sean Moss
 * This file is subject to the terms and conditions of the Microsoft Public License, the text of which can be found in
 * the 'LICENSE' file at the root of this repository, or online at <https://opensource.org/licenses/MS-PL>.
 */

#pragma once

/// Core macros and typedefs for the library

/* Ensure 64-bit */
#if !defined(_M_X64) && !defined(__x86_64__)
#	error Polaris must be built as 64-bit.
#endif

/* OS Detection */
#if defined(_WIN32)
#	define PLSL_WIN32
#	ifndef NOMINMAX
#		define NOMINMAX
#	endif
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#	endif
#elif defined(__APPLE__)
#	include "TargetConditionals.h"
#	if TARGET_OS_OSX==1
#		define PLSL_OSX
#	else
#		error Unsupported Apple OS platform, please use desktop MacOS.
#	endif // TARGET_OS_OSX==1
#	define PLSL_APPLE
#	define PLSL_POSIX
#elif defined(__ANDROID__)
#	error Cannot compile Polaris for Android platforms.
#elif defined(__linux__)
	// Technically android is linux too, but that case is caught above
#	define PLSL_LINUX
#	define PLSL_POSIX
#else
#	error Supported OS not detected - please use Windows, MacOS, or desktop Linux.
#endif // defined(_WIN32)

/* Compiler Detection */
#if defined(_MSC_VER)
#	define PLSL_MSVC
#elif defined(__MINGW32__)
#	error Cannot use MinGW to compile Polaris.
#elif defined(__clang__)
#	define PLSL_CLANG
#elif defined(__GNUC__)
#	define PLSL_GCC
#else
#	error Unsupported compiler detected - please use MSVC, GNU GCC, or Clang.
#endif // defined(_MSC_VER)

/* Import/Export Macros */
#if !defined(PLSL_STATIC)
#	if defined(PLSL_MSVC)
#		if defined(_PLSL_BUILD)
#			define PLSL_API __declspec(dllexport)
#			define PLSL_C_API extern "C" __declspec(dllexport)
#		else
#			define PLSL_API __declspec(dllimport)
#			define PLSL_C_API extern "C" __declspec(dllimport)
#		endif // defined(_PLSL_BUILD)
#	else
#		define PLSL_API __attribute__((__visibility__("default")))
#		define PLSL_C_API extern "C" __attribute__((__visibility__("default")))
#	endif // defined(PLSL_MSVC)
#else
#	define PLSL_API
#endif // !defined(PLSL_STATIC)

/* Library Version */
#define PLSL_VERSION_MAJOR 0
#define PLSL_VERSION_MINOR 1
#define PLSL_VERSION_PATCH 0
#define PLSL_MAKE_VERSION(maj,min,pat) (((maj)<<22)|((min)<<12)|(pat))
#define PLSL_VERSION PLSL_MAKE_VERSION(PLSL_VERSION_MAJOR,PLSL_VERSION_MINOR,PLSL_VERSION_PATCH)

#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cassert>
#include <cstring>

#include <exception>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <functional>

/* Fix some naming issues with GNU C library */
#if defined(major)
#	undef major
#endif
#if defined(minor)
#	undef minor
#endif


namespace pls
{

/* Integer Types */
using uint8  = std::uint8_t;
using uint16 = std::uint16_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;
using int8   = std::int8_t;
using int16  = std::int16_t;
using int32  = std::int32_t;
using int64  = std::int64_t;

/* String */
using string = std::string;
using string_view = std::string_view;

/* Pointer Types */
template<typename T, typename Deleter = std::default_delete<T>>
using uptr = std::unique_ptr<T, Deleter>;
template<typename T>
using sptr = std::shared_ptr<T>;
template<typename T>
using wptr = std::weak_ptr<T>;

/* String Formatting */
template<typename... Args>
inline string mkstr(const char* const fmt, Args&&... args)
{
	static constexpr size_t BUFSIZE = 512;
	char buf[BUFSIZE];
	int len = snprintf(buf, BUFSIZE, fmt, std::forward<Args>(args)...);
	return string(buf, (len < BUFSIZE) ? len : BUFSIZE);
}

} // namespace pls


/* Class Operation Macros */
#define PLSL_NO_COPY(cName) public: cName(const cName&) = delete; cName& operator = (const cName&) = delete;
#define PLSL_NO_MOVE(cName) public: cName(cName&&) = delete; cName& operator = (cName&&) = delete;
#define PLSL_NO_INIT(cName) \
	public: \
	cName() = delete; \
	void* operator new (size_t) = delete; \
	void* operator new[] (size_t) = delete; \
	void* operator new (size_t, const std::nothrow_t&) = delete; \
	void* operator new[] (size_t, const std::nothrow_t&) = delete; \
	void* operator new (size_t, void*) = delete; \
	void* operator new[] (size_t, void*) = delete;
