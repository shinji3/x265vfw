/*****************************************************************************
 * raw.c: raw muxer
 *****************************************************************************
 * Copyright (C) 2003-2015 x264 project
 *
 * Authors: Laurent Aimar <fenrir@via.ecp.fr>
 *          Loren Merritt <lorenm@u.washington.edu>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111, USA.
 *****************************************************************************/

#include "output.h"

static int open_file( char *psz_filename, hnd_t *p_handle, cli_output_opt_t *opt )
{
    if( !strcmp( psz_filename, "-" ) )
        *p_handle = stdout;
    else if( !(*p_handle = x264vfw_fopen( psz_filename, "w+b" )) )
        return -1;

    return 0;
}

static int set_param( hnd_t handle, x265_param *p_param )
{
    return 0;
}

static int write_headers( hnd_t handle, x265_nal *p_nal )
{
    int size = p_nal[0].sizeBytes + p_nal[1].sizeBytes + p_nal[2].sizeBytes;

    if( fwrite( p_nal[0].payload, size, 1, (FILE*)handle ) )
        return size;
    return -1;
}

static int write_frame( hnd_t handle, uint8_t *p_nalu, int i_size, x265_picture *p_picture )
{
    if( fwrite( p_nalu, i_size, 1, (FILE*)handle ) )
        return i_size;
    return -1;
}

static int close_file( hnd_t handle, int64_t largest_pts, int64_t second_largest_pts )
{
    if( !handle || handle == stdout )
        return 0;

    return fclose( (FILE*)handle );
}

const cli_output_t raw_output = { open_file, set_param, write_headers, write_frame, close_file };

