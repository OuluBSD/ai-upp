#pragma once
// U++-style platform/CPU/compiler macros for stdsrc

// Basic type definitions
typedef unsigned char  byte;
typedef unsigned short word;
typedef unsigned int   dword;
typedef unsigned long long qword;

// String type alias
using String = std::string;

// Event type forward declaration
class Event;

// Time type
typedef unsigned long long Time;

// Null constant
#define Null nullptr

// U++-style utility macros
#define UPPCOUNT(x) (sizeof(x) / sizeof((x)[0]))

// Platform detection
#ifdef _WIN32
  #define PLATFORM_WIN32
#elif __APPLE__
  #define PLATFORM_COCOA
#else
  #define PLATFORM_X11
#endif

// Compiler detection
#ifdef __GNUC__
  #define COMPILER_GCC
#elif defined(_MSC_VER)
  #define COMPILER MSC
#elif defined(__clang__)
  #define COMPILER_CLANG
#endif

// CPU detection
#if defined(__x86_64__) || defined(_M_X64)
  #define CPU_AMD64
#elif defined(__i386__) || defined(_M_IX86)
  #define CPU_X86
#elif defined(__arm__) || defined(_M_ARM)
  #define CPU_ARM
#elif defined(__aarch64__)
  #define CPU_ARM64
#endif

// Common constants
#define IDEXIT 9999

// Utility macros
#define GUIPLATFORM_KEYMAP_DECLS
#define GUIPLATFORM_TOPWINDOW_DECLS

// Time constants
#define TIMEID_COUNT 1000