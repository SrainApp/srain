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
    GOutputStream *out;

    out = g_io_stream_get_output_stream(stream);
    // FIXME: if no all data sent, report error
    return g_output_stream_write(out, data, size, NULL, NULL);
}

int io_stream_read(GIOStream *stream, char* buf, size_t size){
    GInputStream *in; 

    in = g_io_stream_get_input_stream(stream);

    return g_input_stream_read(in, buf, size, NULL, NULL);
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
    bool end;
    char byte;
    char prev_byte;

    end = FALSE;
    prev_byte = '\0';
    while (i < size - 1){
        int ret = io_stream_read(stream, &byte, 1);
        if (ret == -1){
            return -1;
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

    return -1;
}
