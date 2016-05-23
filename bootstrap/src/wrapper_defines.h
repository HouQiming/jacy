#line 1 "c:/tp/pure/bin/win32_release/../../wrapper/wrapper_defines.h"
#ifndef __WRAPPER_DEFINES_H
#define __WRAPPER_DEFINES_H
#ifdef LINUX
#undef _WIN32
#endif

#ifndef _WIN32
	#define __declspec(x)
	#define __stdcall
	#define __cdecl
#endif

#ifdef __cplusplus
	#if defined(PM_RELEASE)||!defined(_WIN32)
		#define EXPORT extern "C"
	#else
		#define EXPORT extern "C" __declspec(dllexport)
	#endif
#else
	#if defined(PM_RELEASE)||!defined(_WIN32)
		#define EXPORT
	#else
		#define EXPORT __declspec(dllexport)
	#endif
#endif

#ifdef _M_AMD64
typedef long long iptr;
#else
typedef long iptr;
#endif
#endif
