/*****************************************************************************
 * codec.c: main encoding/decoding functions
 *****************************************************************************
 * Copyright (C) 2003-2016 x265vfw project
 *
 * Authors: Justin Clay
 *          Laurent Aimar <fenrir@via.ecp.fr>
 *          Anton Mitrofanov <BugMaster@narod.ru>
 *          Attila Padar <mpxplay@freemail.hu>
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

#include <assert.h>

#include <getopt.h>

const named_fourcc_t x264vfw_fourcc_table[COUNT_FOURCC] =
{
    { "HEVC", mmioFOURCC('H','E','V','C') },
    { "H265", mmioFOURCC('H','2','6','5') },
    { "h265", mmioFOURCC('h','2','6','5') },
    { "X265", mmioFOURCC('X','2','6','5') },
    { "x265", mmioFOURCC('x','2','6','5') }
};

/* Return a valid x264 colorspace or X264VFW_CSP_NONE if it is not supported */
static int get_csp(BITMAPINFOHEADER *hdr)
{
    /* For YUV the bitmap is always top-down regardless of the biHeight sign */
    int i_vflip = 0;

    switch (hdr->biCompression)
    {
        case FOURCC_I420:
        case FOURCC_IYUV:
            return X264VFW_CSP_I420 | i_vflip;

        case FOURCC_YV12:
            return X264VFW_CSP_YV12 | i_vflip;

        case FOURCC_YV16:
            return X264VFW_CSP_YV16 | i_vflip;

        case FOURCC_YV24:
            return X264VFW_CSP_YV24 | i_vflip;

        case FOURCC_NV12:
            return X264VFW_CSP_NV12 | i_vflip;

        case FOURCC_YUYV:
        case FOURCC_YUY2:
            return X264VFW_CSP_YUYV | i_vflip;

        case FOURCC_UYVY:
        case FOURCC_HDYC:
            return X264VFW_CSP_UYVY | i_vflip;

        case BI_RGB:
        {
            i_vflip = hdr->biHeight < 0 ? 0 : X264VFW_CSP_VFLIP;
            if (hdr->biBitCount == 24)
                return X264VFW_CSP_BGR | i_vflip;
            if (hdr->biBitCount == 32)
                return X264VFW_CSP_BGRA | i_vflip;
            return X264VFW_CSP_NONE;
        }

        default:
            return X264VFW_CSP_NONE;
    }
}

static enum AVPixelFormat csp_to_pix_fmt(int i_csp)
{
    i_csp &= X264VFW_CSP_MASK;
    switch (i_csp)
    {
        case X264VFW_CSP_I420:
        case X264VFW_CSP_YV12:
            return AV_PIX_FMT_YUV420P;

        //case X264VFW_CSP_I422:
        case X264VFW_CSP_YV16:
            return AV_PIX_FMT_YUV422P;

        //case X264VFW_CSP_I444:
        case X264VFW_CSP_YV24:
            return AV_PIX_FMT_YUV444P;

        case X264VFW_CSP_NV12:
            return AV_PIX_FMT_NV12;

        case X264VFW_CSP_YUYV:
            return AV_PIX_FMT_YUYV422;

        case X264VFW_CSP_UYVY:
            return AV_PIX_FMT_UYVY422;

        case X264VFW_CSP_BGR:
            return AV_PIX_FMT_BGR24;

        case X264VFW_CSP_BGRA:
            return AV_PIX_FMT_BGRA;

        default:
            return AV_PIX_FMT_NONE;
    }
}

static int x264vfw_picture_fill(AVPicture *picture, uint8_t *ptr, enum AVPixelFormat pix_fmt, int width, int height)
{
    memset(picture, 0, sizeof(AVPicture));

    switch (pix_fmt)
    {
        case AV_PIX_FMT_YUV420P:
        {
            int size, size2;
            height = (height + 1) & ~1;
            width = (width + 1) & ~1;
            picture->linesize[0] = width;
            picture->linesize[1] =
            picture->linesize[2] = width / 2;
            size  = picture->linesize[0] * height;
            size2 = picture->linesize[1] * height / 2;
            picture->data[0] = ptr;
            picture->data[1] = picture->data[0] + size;
            picture->data[2] = picture->data[1] + size2;
            return size + 2 * size2;
        }

        case AV_PIX_FMT_YUV422P:
        {
            int size, size2;
            width = (width + 1) & ~1;
            picture->linesize[0] = width;
            picture->linesize[1] =
            picture->linesize[2] = width / 2;
            size  = picture->linesize[0] * height;
            size2 = picture->linesize[1] * height;
            picture->data[0] = ptr;
            picture->data[1] = picture->data[0] + size;
            picture->data[2] = picture->data[1] + size2;
            return size + 2 * size2;
        }

        case AV_PIX_FMT_YUV444P:
        {
            int size;
            picture->linesize[0] =
            picture->linesize[1] =
            picture->linesize[2] = width;
            size  = picture->linesize[0] * height;
            picture->data[0] = ptr;
            picture->data[1] = picture->data[0] + size;
            picture->data[2] = picture->data[1] + size;
            return 3 * size;
        }

        case AV_PIX_FMT_NV12:
        {
            int size;
            height = (height + 1) & ~1;
            width = (width + 1) & ~1;
            picture->linesize[0] =
            picture->linesize[1] = width;
            size  = picture->linesize[0] * height;
            picture->data[0] = ptr;
            picture->data[1] = picture->data[0] + size;
            return size + size / 2;
        }

        case AV_PIX_FMT_YUYV422:
        case AV_PIX_FMT_UYVY422:
            width = (width + 1) & ~1;
            picture->linesize[0] = width * 2;
            picture->data[0] = ptr;
            return picture->linesize[0] * height;

        case AV_PIX_FMT_BGR24:
            picture->linesize[0] = (width * 3 + 3) & ~3;
            picture->data[0] = ptr;
            return picture->linesize[0] * height;

        case AV_PIX_FMT_BGRA:
            picture->linesize[0] = width * 4;
            picture->data[0] = ptr;
            return picture->linesize[0] * height;

        default:
            return -1;
    }
}

static int x264vfw_picture_get_size(enum AVPixelFormat pix_fmt, int width, int height)
{
    AVPicture dummy_pict;
    return x264vfw_picture_fill(&dummy_pict, NULL, pix_fmt, width, height);
}

static int x264vfw_picture_vflip(AVPicture *picture, enum AVPixelFormat pix_fmt, int width, int height)
{
    switch (pix_fmt)
    {
        // only RGB-formats can need vflip
        case AV_PIX_FMT_BGR24:
        case AV_PIX_FMT_BGRA:
            picture->data[0] += picture->linesize[0] * (height - 1);
            picture->linesize[0] = -picture->linesize[0];
            break;

        default:
            return -1;
    }
    return 0;
}

static void x264vfw_fill_black_frame(uint8_t *ptr, enum AVPixelFormat pix_fmt, int picture_size)
{
    switch (pix_fmt)
    {
        case AV_PIX_FMT_YUV420P:
        case AV_PIX_FMT_NV12:
        {
            int luma_size = picture_size * 2 / 3;
            memset(ptr, 0x10, luma_size); /* TV Scale */
            memset(ptr + luma_size, 0x80, picture_size - luma_size);
            break;
        }

        case AV_PIX_FMT_YUV422P:
        {
            int luma_size = picture_size / 2;
            memset(ptr, 0x10, luma_size); /* TV Scale */
            memset(ptr + luma_size, 0x80, picture_size - luma_size);
            break;
        }

        case AV_PIX_FMT_YUV444P:
        {
            int luma_size = picture_size / 3;
            memset(ptr, 0x10, luma_size); /* TV Scale */
            memset(ptr + luma_size, 0x80, picture_size - luma_size);
            break;
        }

        case AV_PIX_FMT_YUYV422:
            wmemset((wchar_t *)ptr, 0x8010, picture_size / sizeof(wchar_t)); /* TV Scale */
            break;

        case AV_PIX_FMT_UYVY422:
            wmemset((wchar_t *)ptr, 0x1080, picture_size / sizeof(wchar_t)); /* TV Scale */
            break;

        default:
            memset(ptr, 0x00, picture_size);
            break;
    }
}

static int supported_fourcc(DWORD fourcc)
{
    int i;
    for (i = 0; i < COUNT_FOURCC; i++)
        if (fourcc == x264vfw_fourcc_table[i].value)
            return TRUE;
    return FALSE;
}

LRESULT x264vfw_decompress_get_format(CODEC *codec, BITMAPINFO *lpbiInput, BITMAPINFO *lpbiOutput)
{
    BITMAPINFOHEADER *inhdr = &lpbiInput->bmiHeader;
    BITMAPINFOHEADER *outhdr = &lpbiOutput->bmiHeader;
    int              iWidth;
    int              iHeight;
    int              picture_size;

    if (!lpbiOutput)
        return sizeof(BITMAPINFOHEADER);

    if (!supported_fourcc(inhdr->biCompression))
        return ICERR_BADFORMAT;

    iWidth  = inhdr->biWidth;
    iHeight = inhdr->biHeight;
    if (iWidth <= 0 || iHeight <= 0)
        return ICERR_BADFORMAT;
    /* We need x2 width/height */
    if (iWidth % 2 || iHeight % 2)
        return ICERR_BADFORMAT;

    picture_size = x264vfw_picture_get_size(AV_PIX_FMT_BGRA, iWidth, iHeight);
    if (picture_size < 0)
        return ICERR_BADFORMAT;

    memset(outhdr, 0, sizeof(BITMAPINFOHEADER));
    outhdr->biSize        = sizeof(BITMAPINFOHEADER);
    outhdr->biWidth       = iWidth;
    outhdr->biHeight      = iHeight;
    outhdr->biPlanes      = 1;
    outhdr->biBitCount    = 32;
    outhdr->biCompression = BI_RGB;
    outhdr->biSizeImage   = picture_size;

    return ICERR_OK;
}

LRESULT x264vfw_decompress_query(CODEC *codec, BITMAPINFO *lpbiInput, BITMAPINFO *lpbiOutput)
{
    BITMAPINFOHEADER *inhdr = &lpbiInput->bmiHeader;
    BITMAPINFOHEADER *outhdr = &lpbiOutput->bmiHeader;
    int              iWidth;
    int              iHeight;
    int              i_csp;
    int              picture_size;
    enum AVPixelFormat pix_fmt;

    if (!supported_fourcc(inhdr->biCompression))
        return ICERR_BADFORMAT;

    iWidth  = inhdr->biWidth;
    iHeight = inhdr->biHeight;
    if (iWidth <= 0 || iHeight <= 0)
        return ICERR_BADFORMAT;
    /* We need x2 width/height */
    if (iWidth % 2 || iHeight % 2)
        return ICERR_BADFORMAT;

    if (!lpbiOutput)
        return ICERR_OK;

    if (iWidth != outhdr->biWidth || iHeight != abs(outhdr->biHeight))
        return ICERR_BADFORMAT;

    i_csp = get_csp(outhdr);
    if (i_csp == X264VFW_CSP_NONE)
        return ICERR_BADFORMAT;

    pix_fmt = csp_to_pix_fmt(i_csp);
    if (pix_fmt == AV_PIX_FMT_NONE)
        return ICERR_BADFORMAT;

    picture_size = x264vfw_picture_get_size(pix_fmt, iWidth, iHeight);
    if (picture_size < 0)
        return ICERR_BADFORMAT;

    /* MSDN says that biSizeImage may be set to zero for BI_RGB bitmaps
       But some buggy applications don't set it also for other bitmap types */
    if (outhdr->biSizeImage != 0 && outhdr->biSizeImage < picture_size)
        return ICERR_BADFORMAT;

    return ICERR_OK;
}

LRESULT x264vfw_decompress_begin(CODEC *codec, BITMAPINFO *lpbiInput, BITMAPINFO *lpbiOutput)
{
    int i_csp;

    x264vfw_decompress_end(codec);

    if (x264vfw_decompress_query(codec, lpbiInput, lpbiOutput) != ICERR_OK)
    {
        DPRINTF("incompatible input/output frame format (decode)\n");
        return ICERR_BADFORMAT;
    }

    i_csp = get_csp(&lpbiOutput->bmiHeader);
    codec->decoder_vflip = (i_csp & X264VFW_CSP_VFLIP) != 0;
    i_csp &= X264VFW_CSP_MASK;
    codec->decoder_pix_fmt = csp_to_pix_fmt(i_csp);
    codec->decoder_swap_UV = i_csp == X264VFW_CSP_YV12 || i_csp == X264VFW_CSP_YV16 || i_csp == X264VFW_CSP_YV24;

    codec->decoder = avcodec_find_decoder(AV_CODEC_ID_HEVC);
    if (!codec->decoder)
    {
        DPRINTF("avcodec_find_decoder failed\n");
        return ICERR_ERROR;
    }

    codec->decoder_context = avcodec_alloc_context3(codec->decoder);
    if (!codec->decoder_context)
    {
        DPRINTF("avcodec_alloc_context failed\n");
        return ICERR_ERROR;
    }

    codec->decoder_frame = av_frame_alloc();
    if (!codec->decoder_frame)
    {
        DPRINTF("av_frame_alloc failed\n");
        av_freep(&codec->decoder_context);
        return ICERR_ERROR;
    }

    codec->decoder_context->thread_count = 0; //minimize latency
    codec->decoder_context->coded_width  = lpbiInput->bmiHeader.biWidth;
    codec->decoder_context->coded_height = lpbiInput->bmiHeader.biHeight;
    codec->decoder_context->codec_tag = lpbiInput->bmiHeader.biCompression;

    if (lpbiInput->bmiHeader.biSize > sizeof(BITMAPINFOHEADER) + 4 && lpbiInput->bmiHeader.biSize < (1 << 30))
    {
        uint8_t *buf = (uint8_t *)&lpbiInput->bmiHeader + sizeof(BITMAPINFOHEADER);
        uint32_t buf_size = lpbiInput->bmiHeader.biSize - sizeof(BITMAPINFOHEADER);
        /* Check supported formats of extradata */
        if ((buf[0] == 0x00 && buf[1] == 0x00 && buf[2] == 0x00 && buf[3] == 0x01) ||
            (buf_size >= 7 && buf[0] == 0x01 && (buf[4] & 0xfc) == 0xfc && (buf[5] & 0xe0) == 0xe0))
        {
            codec->decoder_extradata = av_malloc(buf_size + FF_INPUT_BUFFER_PADDING_SIZE);
            if (codec->decoder_extradata)
            {
                codec->decoder_is_avc = buf[0] == 0x01;
                memcpy(codec->decoder_extradata, buf, buf_size);
                memset(codec->decoder_extradata + buf_size, 0, FF_INPUT_BUFFER_PADDING_SIZE);
                codec->decoder_context->extradata = codec->decoder_extradata;
                codec->decoder_context->extradata_size = buf_size;
            }
        }
    }

    if (avcodec_open2(codec->decoder_context, codec->decoder, NULL) < 0)
    {
        DPRINTF("avcodec_open failed\n");
        av_freep(&codec->decoder_context);
        av_frame_free(&codec->decoder_frame);
        av_freep(&codec->decoder_extradata);
        return ICERR_ERROR;
    }

    av_init_packet(&codec->decoder_pkt);
    codec->decoder_pkt.data = NULL;
    codec->decoder_pkt.size = 0;

    return ICERR_OK;
}

/* handle the deprecated jpeg pixel formats */
static int handle_jpeg(int pix_fmt, int *fullrange)
{
    switch (pix_fmt)
    {
        case AV_PIX_FMT_YUVJ420P: *fullrange = 1; return AV_PIX_FMT_YUV420P;
        case AV_PIX_FMT_YUVJ422P: *fullrange = 1; return AV_PIX_FMT_YUV422P;
        case AV_PIX_FMT_YUVJ444P: *fullrange = 1; return AV_PIX_FMT_YUV444P;
        default:                                  return pix_fmt;
    }
}

static struct SwsContext *x264vfw_init_sws_context(CODEC *codec, int dst_width, int dst_height)
{
    struct SwsContext *sws = sws_alloc_context();
    if (!sws)
        return NULL;

    int flags = SWS_BICUBIC |
                SWS_FULL_CHR_H_INP | SWS_ACCURATE_RND;

    int src_width = codec->decoder_context->width;
    int src_height = codec->decoder_context->height;
    if (!src_width || !src_height)
    {
        src_width = codec->decoder_context->coded_width;
        src_height = codec->decoder_context->coded_height;
    }
    int src_range = codec->decoder_context->color_range == AVCOL_RANGE_JPEG;
    int src_pix_fmt = handle_jpeg(codec->decoder_context->pix_fmt, &src_range);

    int dst_range = src_range; //maintain source range
    int dst_pix_fmt = handle_jpeg(codec->decoder_pix_fmt, &dst_range);

    av_opt_set_int(sws, "sws_flags",  flags,       0);

    av_opt_set_int(sws, "srcw",       src_width,   0);
    av_opt_set_int(sws, "srch",       src_height,  0);
    av_opt_set_int(sws, "src_format", src_pix_fmt, 0);
    av_opt_set_int(sws, "src_range",  src_range,   0);

    av_opt_set_int(sws, "dstw",       dst_width,   0);
    av_opt_set_int(sws, "dsth",       dst_height,  0);
    av_opt_set_int(sws, "dst_format", dst_pix_fmt, 0);
    av_opt_set_int(sws, "dst_range",  dst_range,   0);

    /* SWS_FULL_CHR_H_INT is correctly supported only for RGB formats */
    if (dst_pix_fmt == AV_PIX_FMT_BGR24 || dst_pix_fmt == AV_PIX_FMT_BGRA)
        flags |= SWS_FULL_CHR_H_INT;

    const int *coefficients = NULL;
    switch (codec->decoder_context->colorspace)
    {
        case AVCOL_SPC_BT709:
            coefficients = sws_getCoefficients(SWS_CS_ITU709);
            break;
        case AVCOL_SPC_FCC:
            coefficients = sws_getCoefficients(SWS_CS_FCC);
            break;
        case AVCOL_SPC_BT470BG:
            coefficients = sws_getCoefficients(SWS_CS_ITU601);
            break;
        case AVCOL_SPC_SMPTE170M:
            coefficients = sws_getCoefficients(SWS_CS_SMPTE170M);
            break;
        case AVCOL_SPC_SMPTE240M:
            coefficients = sws_getCoefficients(SWS_CS_SMPTE240M);
            break;
        default:
            coefficients = sws_getCoefficients(SWS_CS_DEFAULT);
            break;
    }
    sws_setColorspaceDetails(sws,
                             coefficients, src_range,
                             coefficients, dst_range,
                             0, 1<<16, 1<<16);

    if (sws_init_context(sws, NULL, NULL) < 0)
    {
        sws_freeContext(sws);
        return NULL;
    }
    return sws;
}

LRESULT x264vfw_decompress(CODEC *codec, ICDECOMPRESS *icd)
{
    BITMAPINFOHEADER *inhdr = icd->lpbiInput;
    DWORD neededsize = inhdr->biSizeImage + FF_INPUT_BUFFER_PADDING_SIZE;
    int len, got_picture;
    AVPicture picture;
    int picture_size;

    got_picture = 0;
#if X264VFW_USE_VIRTUALDUB_HACK
    if (!(inhdr->biSizeImage == 1 && ((uint8_t *)icd->lpInput)[0] == 0x7f))
    {
#endif
        /* Check overflow */
        if (neededsize < FF_INPUT_BUFFER_PADDING_SIZE)
        {
            DPRINTF("buffer overflow check failed\n");
            return ICERR_ERROR;
        }
        if (codec->decoder_buf_size < neededsize)
        {
            av_free(codec->decoder_buf);
            codec->decoder_buf_size = 0;
            codec->decoder_buf = av_malloc(neededsize);
            if (!codec->decoder_buf)
            {
                DPRINTF("failed to realloc decoder buffer\n");
                return ICERR_ERROR;
            }
            codec->decoder_buf_size = neededsize;
        }
        memcpy(codec->decoder_buf, icd->lpInput, inhdr->biSizeImage);
        memset(codec->decoder_buf + inhdr->biSizeImage, 0, FF_INPUT_BUFFER_PADDING_SIZE);
        codec->decoder_pkt.data = codec->decoder_buf;
        codec->decoder_pkt.size = inhdr->biSizeImage;

        if (inhdr->biSizeImage >= 4 && !codec->decoder_is_avc)
        {
            uint8_t *buf = codec->decoder_buf;
            uint32_t buf_size = inhdr->biSizeImage;
            uint32_t nal_size = endian_fix32(*(uint32_t *)buf);
            /* Check startcode */
            if (nal_size != 0x00000001)
            {
                /* Check that this is correct size prefixed format */
                while ((uint64_t)buf_size >= (uint64_t)nal_size + 8)
                {
                    buf += nal_size + 4;
                    buf_size -= nal_size + 4;
                    nal_size = endian_fix32(*(uint32_t *)buf);
                }
                if ((uint64_t)buf_size == (uint64_t)nal_size + 4)
                {
                    /* Convert to Annex B */
                    buf = codec->decoder_buf;
                    buf_size = inhdr->biSizeImage;
                    nal_size = endian_fix32(*(uint32_t *)buf);
                    *(uint32_t *)buf = endian_fix32(0x00000001);
                    while ((uint64_t)buf_size >= (uint64_t)nal_size + 8)
                    {
                        buf += nal_size + 4;
                        buf_size -= nal_size + 4;
                        nal_size = endian_fix32(*(uint32_t *)buf);
                        *(uint32_t *)buf = endian_fix32(0x00000001);
                    }
                }
            }
        }

        len = avcodec_decode_video2(codec->decoder_context, codec->decoder_frame, &got_picture, &codec->decoder_pkt);
        if (len < 0)
        {
            DPRINTF("avcodec_decode_video2 failed\n");
            return ICERR_ERROR;
        }
#if X264VFW_USE_VIRTUALDUB_HACK
    }
#endif

    picture_size = x264vfw_picture_get_size(codec->decoder_pix_fmt, inhdr->biWidth, inhdr->biHeight);
    if (picture_size < 0)
    {
        DPRINTF("x264vfw_picture_get_size failed\n");
        return ICERR_ERROR;
    }

    if (!got_picture)
    {
        /* Frame was decoded but delayed so we would show the BLACK-frame instead */
        x264vfw_fill_black_frame(icd->lpOutput, codec->decoder_pix_fmt, picture_size);
        //icd->lpbiOutput->biSizeImage = picture_size;
        return ICERR_OK;
    }

    if (x264vfw_picture_fill(&picture, icd->lpOutput, codec->decoder_pix_fmt, inhdr->biWidth, inhdr->biHeight) < 0)
    {
        DPRINTF("x264vfw_picture_fill failed\n");
        return ICERR_ERROR;
    }
    if (codec->decoder_swap_UV)
    {
        uint8_t *temp_data;
        int     temp_linesize;

        temp_data = picture.data[1];
        temp_linesize = picture.linesize[1];
        picture.data[1] = picture.data[2];
        picture.linesize[1] = picture.linesize[2];
        picture.data[2] = temp_data;
        picture.linesize[2] = temp_linesize;
    }
    if (codec->decoder_vflip)
        if (x264vfw_picture_vflip(&picture, codec->decoder_pix_fmt, inhdr->biWidth, inhdr->biHeight) < 0)
        {
            DPRINTF("x264vfw_picture_vflip failed\n");
            return ICERR_ERROR;
        }

    if (!codec->sws)
    {
        codec->sws = x264vfw_init_sws_context(codec, inhdr->biWidth, inhdr->biHeight);
        if (!codec->sws)
        {
            DPRINTF("x264vfw_init_sws_context failed\n");
            return ICERR_ERROR;
        }
    }

    sws_scale(codec->sws, (const uint8_t * const *)codec->decoder_frame->data, codec->decoder_frame->linesize, 0, inhdr->biHeight, picture.data, picture.linesize);
    //icd->lpbiOutput->biSizeImage = picture_size;

    return ICERR_OK;
}

LRESULT x264vfw_decompress_end(CODEC *codec)
{
    codec->decoder_is_avc = 0;
    if (codec->decoder_context)
        avcodec_close(codec->decoder_context);
    av_freep(&codec->decoder_context);
    av_frame_free(&codec->decoder_frame);
    av_freep(&codec->decoder_extradata);
    av_freep(&codec->decoder_buf);
    codec->decoder_buf_size = 0;
    sws_freeContext(codec->sws);
    codec->sws = NULL;
    return ICERR_OK;
}
