/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2014 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#ifndef _SDL_config_h
#define _SDL_config_h

#include "SDL_platform.h"

#define HAVE_LIBC
#define SDL_AUDIO_DISABLED  1
#define SDL_JOYSTICK_DISABLED   1
#define SDL_HAPTIC_DISABLED 1
#define SDL_LOADSO_DISABLED 1
#define SDL_FILESYSTEM_DUMMY  1
#define SDL_RENDER_DISABLED 1

/**
 *  \file SDL_config.h
 */

/* Add any platform that doesn't build using the configure system. */
#ifdef USING_PREMAKE_CONFIG_H
#include "SDL_config_premake.h"
#elif defined(__WIN32__)
#include "SDL_config_windows.h"
#elif defined(__WINRT__)
#include "SDL_config_winrt.h"
#elif defined(__MACOSX__)
#include "SDL_config_macosx.h"
#elif defined(__IPHONEOS__)
#include "SDL_config_iphoneos.h"
#elif defined(__ANDROID__)
#include "SDL_config_android.h"
#elif defined(__PSP__)
#include "SDL_config_psp.h"
#elif defined(JC_SDL_USE_LINUX)
#include <wchar.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>
#define SDL_LOADSO_DLOPEN 1
/* Enable various threading systems */
#define SDL_THREAD_PTHREAD 1
#define SDL_THREAD_PTHREAD_RECURSIVE_MUTEX_NP 1
/* Enable various timer systems */
#define SDL_TIMER_UNIX 1
/* Enable various video drivers */
#define SDL_VIDEO_DRIVER_X11 1
#define SDL_VIDEO_DRIVER_X11_DPMS 1
#define SDL_VIDEO_DRIVER_X11_XINERAMA 1
#define SDL_VIDEO_DRIVER_X11_XME 1
#define SDL_MAIN_DUMMY 1
#define SDL_VIDEO_DRIVER_X11_DYNAMIC "libX11.so.6"
#define SDL_VIDEO_DRIVER_X11_DYNAMIC_XEXT "libXext.so.6"
#define SDL_VIDEO_DRIVER_X11_DYNAMIC_XRANDR "libXrandr.so.2"
#define SDL_VIDEO_DRIVER_X11_DYNAMIC_XRENDER "libXrender.so.1"
#define SDL_VIDEO_DRIVER_X11_SUPPORTS_GENERIC_EVENTS 1
#else
/* This is a minimal configuration just to get SDL running on new platforms */
#include "SDL_config_minimal.h"
#endif /* platform config */

#ifdef USING_GENERATED_CONFIG_H
#error Wrong SDL_config.h, check your include path?
#endif

#define HAVE_LIBC
#define SDL_AUDIO_DISABLED  1
#define SDL_JOYSTICK_DISABLED   1
#define SDL_HAPTIC_DISABLED 1
#define SDL_LOADSO_DISABLED 1
#define SDL_FILESYSTEM_DUMMY  1
#define SDL_RENDER_DISABLED 1
#undef SDL_VIDEO_DRIVER_DUMMY
#define SDL_VIDEO_DRIVER_DUMMY 0

#define SDL_malloc malloc
#define SDL_calloc calloc
#define SDL_realloc realloc
#define SDL_free free
#define SDL_acos acos
#define SDL_asin asin
#define SDL_atan atan
#define SDL_atan2 atan2
#define SDL_ceil ceil
#define SDL_copysign copysign
#define SDL_cos cos
#define SDL_cosf cosf
#define SDL_fabs fabs
#define SDL_floor floor
#define SDL_log log
#define SDL_pow pow
#define SDL_scalbn scalbn
#define SDL_sin sin
#define SDL_sinf sinf
#define SDL_sqrt sqrt
#define SDL_memset memset
#define SDL_memcpy memcpy
#define SDL_memmove memmove
#define SDL_memcmp memcmp

#endif /* _SDL_config_h */
