#ifndef _BUFFER_H
#define _BUFFER_H

/* src: https://github.com/reagent/buffer */

#define __STDC_WANT_LIB_EXT2__ 1
#define _GNU_SOURCE

#include <stddef.h>

typedef struct buffer buffer;

buffer *new_buffer(size_t size);
size_t buffer_length(buffer *buffer);
size_t buffer_size(buffer *buffer);
int buffer_append(buffer *buffer, const char *src, size_t length);
int buffer_appendf(buffer *buffer, const char *format, ...);
int buffer_nappendf(buffer *buffer, size_t length, const char *format, ...);
char *buffer_to_string(buffer *buffer);
void buffer_free(buffer *buffer);

#endif
