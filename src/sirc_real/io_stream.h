#ifndef __IO_STREAM_H
#define __IO_STREAM_H

#include <gio/gio.h>

int io_stream_write(GIOStream *stream, const char* data, size_t size);
int io_stream_read(GIOStream *stream, char* buf, size_t size);
int io_stream_readline(GIOStream *stream, char *buf, size_t size);

#endif
