#ifndef _BUFFER_H
#define _BUFFER_H

#define __STDC_WANT_LIB_EXT2__ 1
#define _GNU_SOURCE

#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

typedef struct buffer {
        char *data;
        uint length;
        uint size;
} buffer;

buffer *new_buffer(uint size);
int buffer_length(buffer *buffer);
int buffer_size(buffer *buffer);
int buffer_append(buffer *buffer, const char *src, uint length);
int buffer_appendf(buffer *buffer, const char *format, ...);
int buffer_nappendf(buffer *buffer, size_t length, const char *format, ...);
char *buffer_to_string(buffer *buffer);
void buffer_free(buffer *buffer);

#endif