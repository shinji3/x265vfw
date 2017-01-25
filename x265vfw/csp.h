/*****************************************************************************
 * csp.h: colorspace conversion functions
 *****************************************************************************
 * Copyright (C) 2004-2016 x264vfw project
 *
 * Authors: Laurent Aimar <fenrir@via.ecp.fr>
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

#ifndef X264VFW_CSP_H
#define X264VFW_CSP_H

#include "common.h"

/* Colorspace type */
#define X264VFW_CSP_MASK           0x000F
#define X264VFW_CSP_I400           0    /* 0 yuv 4:0:0 planar */
#define X264VFW_CSP_I420           1    /* 1 yuv 4:2:0 planar */
#define X264VFW_CSP_I422           2    /* 2 yuv 4:2:2 planar */
#define X264VFW_CSP_I444           3    /* 3 yuv 4:4:4 planar */
#define X264VFW_CSP_NV12           4    /* 4 yuv 4:2:0, with one y plane and one packed u+v */
#define X264VFW_CSP_NV16           5    /* 5 yuv 4:2:2, with one y plane and one packed u+v */
#define X264VFW_CSP_BGR            6    /* 6 packed bgr 24bits */
#define X264VFW_CSP_BGRA           7    /* 7 packed bgr 32bits */
#define X264VFW_CSP_RBG            8    /* 8 packed rgb 24bits   */

#define X264VFW_CSP_YV12           0x0009  /* yvu 4:2:0 planar */
#define X264VFW_CSP_YUYV           0x000A  /* yuv 4:2:2 packed */
#define X264VFW_CSP_UYVY           0x000B  /* yuv 4:2:2 packed */
#define X264VFW_CSP_YV16           0x000C  /* yvu 4:2:2 planar */
#define X264VFW_CSP_YV24           0x000D  /* yvu 4:4:4 planar */
#define X264VFW_CSP_NONE           0x000E  /* invalid mode */
#define X264VFW_CSP_MAX            0x000F  /* end of list */
#define X264VFW_CSP_VFLIP          0x1000  /* the csp is vertically flipped */

#endif
