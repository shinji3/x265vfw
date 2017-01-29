/*****************************************************************************
 * driverproc.c: vfw wrapper
 *****************************************************************************
 * Copyright (C) 2003-2016 x265vfw project
 *
 * Authors: Justin Clay
 *          Laurent Aimar <fenrir@via.ecp.fr>
 *          Anton Mitrofanov <BugMaster@narod.ru>
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

#include "x265vfw.h"

#ifdef PTW32_STATIC_LIB
#include <pthread.h>
#endif

/* Global DLL instance */
HINSTANCE x264vfw_hInst;
/* Global DLL critical section */
CRITICAL_SECTION x264vfw_CS;

/* Calling back point for our DLL so we can keep track of the window in x264vfw_hInst */
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    x264vfw_hInst = hinstDLL;

#ifdef PTW32_STATIC_LIB
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            InitializeCriticalSection(&x264vfw_CS);
            pthread_win32_process_attach_np();
            pthread_win32_thread_attach_np();
            break;

        case DLL_THREAD_ATTACH:
            pthread_win32_thread_attach_np();
            break;

        case DLL_THREAD_DETACH:
            pthread_win32_thread_detach_np();
            break;

        case DLL_PROCESS_DETACH:
            pthread_win32_thread_detach_np();
            pthread_win32_process_detach_np();
            DeleteCriticalSection(&x264vfw_CS);
            break;
    }
#else
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            InitializeCriticalSection(&x264vfw_CS);
            break;

        case DLL_PROCESS_DETACH:
            DeleteCriticalSection(&x264vfw_CS);
            break;
    }
#endif

    return TRUE;
}

#if defined(HAVE_FFMPEG)
static void log_callback(void *ptr, int level, const char *fmt, va_list vl)
{
    if (level <= av_log_get_level())
        DVPRINTF(fmt, vl);
}
#endif

/* This little puppy handles the calls which VFW programs send out to the codec */
LRESULT WINAPI attribute_align_arg DriverProc(DWORD_PTR dwDriverId, HDRVR hDriver, UINT uMsg, LPARAM lParam1, LPARAM lParam2)
{
    CODEC *codec = (CODEC *)dwDriverId;
    ICDECOMPRESSEX *px;

    switch (uMsg)
    {
        case DRV_LOAD:
#if defined(HAVE_FFMPEG)
            avcodec_register_all();
            av_log_set_callback(log_callback);
#endif
            return DRV_OK;

        case DRV_FREE:
            return DRV_OK;

        case DRV_OPEN:
        {
            ICOPEN *icopen = (ICOPEN *)lParam2;

            if (icopen && icopen->fccType != ICTYPE_VIDEO)
                return 0;

            if (!(codec = malloc(sizeof(CODEC))))
            {
                if (icopen)
                    icopen->dwError = ICERR_MEMORY;
                return 0;
            }

            memset(codec, 0, sizeof(CODEC));

            if (icopen)
                icopen->dwError = ICERR_OK;
            return (LRESULT)codec;
        }

        case DRV_CLOSE:
            /* From xvid: x264vfw_compress_end/x264vfw_decompress_end don't always get called */
#if defined(HAVE_FFMPEG) && X264VFW_USE_DECODER
            x264vfw_decompress_end(codec);
#endif
            free(codec);
            return DRV_OK;

        case DRV_QUERYCONFIGURE:
            return 0;

        case DRV_CONFIGURE:
            return DRV_CANCEL;

/*
        case DRV_DISABLE:
        case DRV_ENABLE:
        case DRV_INSTALL:
        case DRV_REMOVE:
        case DRV_EXITSESSION:
        case DRV_POWER:
            return DRV_OK;
*/

        /* ICM */
        case ICM_GETSTATE:
            return ICERR_OK;

        case ICM_SETSTATE:
            return 0;

        case ICM_GETINFO:
        {
            ICINFO *icinfo = (ICINFO *)lParam1;

            if (lParam2 < sizeof(ICINFO))
                return 0;

            /* Fill all members of the ICINFO structure except szDriver */
            icinfo->dwSize       = sizeof(ICINFO);
            icinfo->fccType      = ICTYPE_VIDEO;
            icinfo->fccHandler   = FOURCC_X264;
            icinfo->dwFlags      = VIDCF_COMPRESSFRAMES | VIDCF_FASTTEMPORALC;
#if defined(HAVE_FFMPEG) && X264VFW_USE_DECODER
            /* ICM_GETINFO may be called before DRV_OPEN so 'codec' can point to NULL */
            icinfo->dwFlags |= VIDCF_FASTTEMPORALD;
#endif
            icinfo->dwVersion    = 0;
#ifdef ICVERSION
            icinfo->dwVersionICM = ICVERSION;
#else
            icinfo->dwVersionICM = 0x0104; /* MinGW's vfw.h doesn't define ICVERSION for some weird reason */
#endif
            wcscpy(icinfo->szName, X264VFW_NAME_L);
            wcscpy(icinfo->szDescription, X264VFW_DESC_L);

            return sizeof(ICINFO);
        }

        case ICM_GET:
            if (!(void *)lParam1)
                return 0;
            return ICERR_OK;

        case ICM_SET:
            return 0;

#if defined(HAVE_FFMPEG) && X264VFW_USE_DECODER
        /* Decompressor */
        case ICM_DECOMPRESS_GET_FORMAT:
            return x264vfw_decompress_get_format(codec, (BITMAPINFO *)lParam1, (BITMAPINFO *)lParam2);

        case ICM_DECOMPRESS_QUERY:
            return x264vfw_decompress_query(codec, (BITMAPINFO *)lParam1, (BITMAPINFO *)lParam2);

        case ICM_DECOMPRESS_BEGIN:
            return x264vfw_decompress_begin(codec, (BITMAPINFO *)lParam1, (BITMAPINFO *)lParam2);

        case ICM_DECOMPRESS:
            return x264vfw_decompress(codec, (ICDECOMPRESS *)lParam1);

        case ICM_DECOMPRESS_END:
            return x264vfw_decompress_end(codec);

        case ICM_DECOMPRESSEX_QUERY:
            px = (ICDECOMPRESSEX *)lParam1;
            return x264vfw_decompress_query(codec, (BITMAPINFO *)px->lpbiSrc, (BITMAPINFO *)px->lpbiDst);

        case ICM_DECOMPRESSEX_BEGIN:
            px = (ICDECOMPRESSEX *)lParam1;
            return x264vfw_decompress_begin(codec, (BITMAPINFO *)px->lpbiSrc, (BITMAPINFO *)px->lpbiDst);

        case ICM_DECOMPRESSEX:
            return x264vfw_decompress(codec, (ICDECOMPRESS *)lParam1);

        case ICM_DECOMPRESSEX_END:
            return x264vfw_decompress_end(codec);
#endif

        default:
            if (uMsg < DRV_USER)
                return DefDriverProc(dwDriverId, hDriver, uMsg, lParam1, lParam2);
            else
                return ICERR_UNSUPPORTED;
    }
}
