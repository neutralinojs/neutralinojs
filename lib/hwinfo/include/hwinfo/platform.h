#pragma once

#if defined(unix) || defined(__unix) || defined(__unix__)
#define HWINFO_UNIX
#endif
#if defined(__APPLE__)
#define HWINFO_APPLE
#endif
#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#define HWINFO_WINDOWS
#define NOMINMAX
#endif

#if defined(__x86_64__) || defined(__x86_64) || defined(__amd64__) || defined(_M_X64)
#define HWINFO_X86_64
#elif defined(__i386__) || defined(_M_IX86)
#define HWINFO_X86_32
#endif
#if defined(HWINFO_X86_64) || defined(HWINFO_X86_32)
#define HWINFO_X86
#endif

#if defined(__arm__) || defined(_M_ARM)
#define HWINFO_ARM32
#elif defined(__aarch64__) || defined(_M_ARM64)
#define HWINFO_AARCH64
#endif

#if defined(HWINFO_ARM32) || defined(HWINFO_AARCH64)
#define HWINFO_ARM
#endif

// dll exports/imports for windows shared libraries
#ifdef _WIN32
#ifdef HWINFO_EXPORTS
#define HWINFO_API __declspec(dllexport)
#else
#define HWINFO_API __declspec(dllimport)
#endif
#pragma warning(disable : 4251)
#else
#define HWINFO_API __attribute__((visibility("default")))
#endif

#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(nodiscard)
#define HWI_NODISCARD [[nodiscard]]
#endif
#endif

#ifndef HWI_NODISCARD
#define HWI_NODISCARD
#endif