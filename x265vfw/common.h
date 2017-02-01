/*****************************************************************************
 * common.h: misc common functions
 *****************************************************************************
 * Copyright (C) 2010-2016 x264vfw project
 *
 * Authors: Anton Mitrofanov <BugMaster@narod.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.
 *****************************************************************************/

#ifndef X264VFW_COMMON_H
#define X264VFW_COMMON_H

#include "x265vfw_config.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <windows.h>
#include <wchar.h>

#include "config.h"

#ifdef HAVE_STDINT_H
#include <stdint.h>
#else
#include <inttypes.h>
#endif

#ifdef _MSC_VER
#define inline __inline
#else
#include <strings.h>
#endif

#if defined(__GNUC__) && (__GNUC__ > 3 || __GNUC__ == 3 && __GNUC_MINOR__ > 0)
#define ALWAYS_INLINE __attribute__((always_inline)) inline
#else
#ifdef _MSC_VER
#define ALWAYS_INLINE __forceinline
#else
#define ALWAYS_INLINE inline
#endif
#endif

#if defined(__GNUC__) && (__GNUC__ > 4 || __GNUC__ == 4 && __GNUC_MINOR__>1)
#define attribute_align_arg __attribute__((force_align_arg_pointer))
#else
#define attribute_align_arg
#endif

#define asm __asm__

#if WORDS_BIGENDIAN
#define endian_fix32(x) (x)
#elif HAVE_X86_INLINE_ASM && HAVE_MMX
static ALWAYS_INLINE uint32_t endian_fix32( uint32_t x )
{
    asm("bswap %0":"+r"(x));
    return x;
}
#elif defined(__GNUC__) && HAVE_ARMV6
static ALWAYS_INLINE uint32_t endian_fix32( uint32_t x )
{
    asm("rev %0, %0":"+r"(x));
    return x;
}
#else
static ALWAYS_INLINE uint32_t endian_fix32( uint32_t x )
{
    return (x<<24) + ((x<<8)&0xff0000) + ((x>>8)&0xff00) + (x>>24);
}
#endif

#if X264VFW_DEBUG_OUTPUT
#define DPRINTF_BUF_SZ 2048
static inline void DPRINTF(const char *fmt, ...)
{
    va_list arg;
    char buf[DPRINTF_BUF_SZ];

    va_start(arg, fmt);
    memset(buf, 0, sizeof(buf));
    vsnprintf(buf, sizeof(buf) - 1, fmt, arg);
    va_end(arg);
    OutputDebugString(buf);
}
static inline void DVPRINTF(const char *fmt, va_list arg)
{
    char buf[DPRINTF_BUF_SZ];

    memset(buf, 0, sizeof(buf));
    vsnprintf(buf, sizeof(buf) - 1, fmt, arg);
    OutputDebugString(buf);
}
#else
static inline void DPRINTF(const char *fmt, ...) {}
static inline void DVPRINTF(const char *fmt, va_list arg) {}
#endif

#endif
