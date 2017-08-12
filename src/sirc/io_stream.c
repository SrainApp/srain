/* Copyright (C) 2016-2017 Shengyu Zhang <srain@srain.im>
 *
 * This file is part of Srain.
 *
 * Srain is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

 /**
 * @file io_stream.c
 * @brief Unused GIOStream wrapper
 * @author Shengyu Zhang <srain@srain.im>
 * @version 0.06
 * @date 2017-04-11
 *
 */


#include <gio/gio.h>
#include <glib.h>

#include "srain.h"
#include "log.h"

int io_stream_write(GIOStream *stream, const char* data, size_t size){
    int ret;
    GOutputStream *out;

    g_return_val_if_fail(!g_io_stream_is_closed(stream), SRN_ERR);

    out = g_io_stream_get_output_stream(stream);
    // FIXME: if no all data sent, report error
    ret =  g_output_stream_write(out, data, size, NULL, NULL);
    if (ret < 0) ret = SRN_ERR;

    return ret;
}

int io_stream_read(GIOStream *stream, char* buf, size_t size){
    int ret;
    GInputStream *in; 

    g_return_val_if_fail(!g_io_stream_is_closed(stream), SRN_ERR);

    in = g_io_stream_get_input_stream(stream);

    ret = g_input_stream_read(in, buf, size, NULL, NULL);
    if (ret < 0) ret = SRN_ERR;

    return ret;
}

/**
 * @brief Read a line (ends with "\r\n") from GIOStream, "\r\n" will not be
 *        contained in the returned buffer.
 *
 * @param stream
 * @param buf
 * @param size
 *
 * @return Number of bytes read, or -1 on error
 */
int io_stream_readline(GIOStream *stream, char *buf, size_t size){
    int i = 0;
    char byte;
    char prev_byte;

    prev_byte = '\0';
    while (i < size - 1){
        int ret = io_stream_read(stream, &byte, 1);
        if (ret == SRN_ERR){
            return ret;
        }
        /* Read "\r\n" */
       if (prev_byte == '\r' && byte == '\n'){
           buf[i-1] = '\0'; // Clear previous '\r'
           return i;
       }

       prev_byte = byte;
       buf[i++] = byte;
    }

    WARN_FR("Length of the line exceeds the buffer");

    return SRN_ERR;
}
