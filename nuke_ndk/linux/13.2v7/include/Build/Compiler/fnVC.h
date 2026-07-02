// Copyright (c) 2009 The Foundry Visionmongers Ltd.  All Rights Reserved.

#ifndef __fnVC_h__
#define __fnVC_h__

#include <FLOAT.H>

#ifndef FN_SHARED_EXPORT
#define FN_SHARED_EXPORT __declspec(dllexport)
#endif

#define mFnSharedObjectExport FN_SHARED_EXPORT 

#ifndef FN_SHARED_IMPORT
#define FN_SHARED_IMPORT __declspec(dllimport)
#endif

#ifndef FN_NOINLINE
// Force non-inline functions
#define FN_NOINLINE __declspec(noinline)
#endif

#ifndef NAME_MAX
  #define NAME_MAX ( _MAX_FNAME + _MAX_EXT )
#endif

#ifndef PATH_MAX
  #define PATH_MAX ( 1024 )
#endif

#ifndef _MT
  #error TURN ON THE MULTITHREADED FLAG
#endif

#ifdef __cplusplus
  #ifndef _CPPUNWIND
    #error TURN ON EXCEPTION HANDLING
  #endif // _CPPUNWIND
#endif // __cplusplus

/* force w2k api to be used: */
#if WINVER < 0x0500
  #define WINVER 0x500
#endif
#if _WIN32_WINNT < 0x500
  #define _WIN32_WINNT 0x500
#endif
#include <windows.h>

//VC++ warn you when an overload is resolved by argument type, i.e.
//void f(int a) {a++;}
//void f(float a) {a++;}
//and you call f(int(0)), you'll get a warning. AS long as it's the right thing, who cares?
#pragma warning( disable : 4675 )

#ifndef NOMINMAX
  #define NOMINMAX
#endif

#ifdef max
  #undef max
#endif

#ifdef min
  #undef min
#endif

#ifndef _USE_MATH_DEFINES

#ifndef M_PI
  #define M_PI                3.14159265358979323846
#endif
#ifndef M_PI_2
  #define M_PI_2              1.57079632679489661923
#endif
#ifndef M_PI_4
  #define M_PI_4              0.78539816339744830962
#endif
#ifndef M_SQRT2
  #define M_SQRT2             1.41421356237309504880
#endif
#ifndef M_E
  #define M_E     2.7182818284590452354
#endif

#endif

#ifndef mFnAlign
#define mFnAlign(ALIGNMENT) __declspec(align(ALIGNMENT))
#endif

/* Function Deprecation macro
 * The message parameter should be a string detailing how to replace the function.
 */
#define mFnDeprecatedFunc(message, func) __declspec(deprecated(message)) func

#endif // __fnVC_h__

