/*****************************************************************************
 * x265vfw.h: x265vfw main header
 *****************************************************************************
 * Copyright (C) 2003-2016 x264vfw project
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

#ifndef X264VFW_X264VFW_H
#define X264VFW_X264VFW_H

#include "common.h"
#include <vfw.h>

#if defined(HAVE_FFMPEG)
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#endif

#include "csp.h"

/* Name */
#define X264VFW_NAME_L L"x265vfw"
#define X264VFW_DESC_L L"x265vfw - H.265/MPEG-H codec"

/* Codec FourCC */
#define FOURCC_X264 mmioFOURCC('X','2','6','5')

/* YUV 4:2:0 planar */
#define FOURCC_I420 mmioFOURCC('I','4','2','0')
#define FOURCC_IYUV mmioFOURCC('I','Y','U','V')
#define FOURCC_YV12 mmioFOURCC('Y','V','1','2')
/* YUV 4:2:2 planar */
#define FOURCC_YV16 mmioFOURCC('Y','V','1','6')
/* YUV 4:4:4 planar */
#define FOURCC_YV24 mmioFOURCC('Y','V','2','4')
/* YUV 4:2:0, with one Y plane and one packed U+V */
#define FOURCC_NV12 mmioFOURCC('N','V','1','2')
/* YUV 4:2:2 packed */
#define FOURCC_YUYV mmioFOURCC('Y','U','Y','V')
#define FOURCC_YUY2 mmioFOURCC('Y','U','Y','2')
#define FOURCC_UYVY mmioFOURCC('U','Y','V','Y')
#define FOURCC_HDYC mmioFOURCC('H','D','Y','C')

#define COUNT_FOURCC     5

/* Types */
typedef struct
{
    const char * const name;
    const DWORD value;
} named_fourcc_t;

/* CODEC: VFW codec instance */
typedef struct
{
    /* Decoder */
#if defined(HAVE_FFMPEG) && X264VFW_USE_DECODER
    int                decoder_is_avc;
    AVCodec            *decoder;
    AVCodecContext     *decoder_context;
    AVFrame            *decoder_frame;
    void               *decoder_extradata;
    void               *decoder_buf;
    DWORD              decoder_buf_size;
    AVPacket           decoder_pkt;
    enum AVPixelFormat decoder_pix_fmt;
    int                decoder_vflip;
    int                decoder_swap_UV;
    struct SwsContext  *sws;
#endif
} CODEC;

#if defined(HAVE_FFMPEG) && X264VFW_USE_DECODER
/* Decompress functions */
LRESULT x264vfw_decompress_get_format(CODEC *, BITMAPINFO *, BITMAPINFO *);
LRESULT x264vfw_decompress_query(CODEC *, BITMAPINFO *, BITMAPINFO *);
LRESULT x264vfw_decompress_begin(CODEC *, BITMAPINFO *, BITMAPINFO *);
LRESULT x264vfw_decompress(CODEC *, ICDECOMPRESS *);
LRESULT x264vfw_decompress_end(CODEC *);
#endif

/* DLL instance */
extern HINSTANCE x264vfw_hInst;
/* DLL critical section */
extern CRITICAL_SECTION x264vfw_CS;

#endif
