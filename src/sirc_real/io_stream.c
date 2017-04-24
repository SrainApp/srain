 /**
 * @file io_stream.c
 * @brief GIOStream wrapper
 * @author Shengyu Zhang <silverrainz@outlook.com>
 * @version 1.0
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
