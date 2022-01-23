#include "buffer.h"

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum error_buffer {
        ERROR_NEW_BUFFER,
        ERROR_BUFFER_APPEND,
        ERROR_BUFFER_TO_STRING
};

static char *error_message[] = {
        "failed to create new buffer.",
        "failed to append data to buffer.",
        "failed to create string from buffer."
};

typedef struct buffer {
        char   *data;
        size_t length;
        size_t size;
} buffer;

buffer *
new_buffer(size_t size)
{
        assert(size > 0);

        buffer *new_buffer = malloc(sizeof(struct buffer));
        if (new_buffer == NULL) {
                fprintf(stderr, "ERROR: %s - (%s)\n",
                        error_message[ERROR_NEW_BUFFER],
                        strerror(errno));

                return NULL;
        }

        char *new_buffer_data = calloc(1, size * sizeof(char));
        if (new_buffer_data == NULL) {
                fprintf(stderr, "ERROR: %s - (%s)\n",
                        error_message[ERROR_NEW_BUFFER],
                        strerror(errno));

                free(new_buffer);

                return NULL;
        }

        *new_buffer = (struct buffer) {
                .data = new_buffer_data,
                .length = 0,
                .size = size
        };

        return new_buffer;
}

size_t
buffer_length(buffer *buffer)
{
        assert(buffer != NULL);

        return buffer->length;
}

size_t
buffer_size(buffer *buffer)
{
        assert(buffer != NULL);

        return buffer->size;
}

static int
buffer_has_storage_available(buffer *buffer, size_t length)
{
        assert(buffer != NULL && length > 0);

        const uint available_storage = buffer->size - buffer->length;

        return length <= available_storage;
}

static int
increase_storage_capacity(buffer *buffer, size_t size)
{
        assert(buffer != NULL && size > 0);

        const size_t buffer_size = buffer->size;
        const size_t new_size = buffer_size < size ?
                                (size * 2) : (buffer_size * 2);

        char *new_buffer_data = realloc(buffer->data, new_size * sizeof(char));
        if (new_buffer_data == NULL) {
                return -1;
        }

        *buffer = (struct buffer) {
                .data = new_buffer_data,
                .size = new_size,
                .length = buffer->length
        };

        return 0;
}

int
buffer_append(buffer *buffer, const char *src, size_t length)
{
        assert(buffer != NULL && src != NULL && length > 0);

        const size_t safe_length = length + 1;

        if (!buffer_has_storage_available(buffer, safe_length)) {
                if (increase_storage_capacity(buffer, safe_length) < 0) {
                        fprintf(stderr, "ERROR: %s - (%s)\n",
                                error_message[ERROR_BUFFER_APPEND],
                                strerror(errno));

                        return -1;
                }
        }

        strncat(buffer->data, src, length);
        buffer->length += length;

        return 0;
}

int
buffer_appendf(buffer *buffer, const char *format, ...)
{
        assert(buffer != NULL && format != NULL);

        va_list args;
        va_start(args, format);

        char *temp = NULL;
        int length = vasprintf(&temp, format, args);
        if (length < 0) {
                fprintf(stderr, "ERROR: %s - (vasprintf() return: %d)\n",
                        error_message[ERROR_BUFFER_APPEND],
                        length);

                if (temp != NULL) {
                        free(temp);
                }

                return -1;
        }

        va_end(args);

        int status = 0;
        status = buffer_append(buffer, temp, length);
        if (status < 0) {
                if (temp != NULL) {
                        free(temp);
                }

                return -1;
        }

        free(temp);

        return 0;
}

int
buffer_nappendf(buffer *buffer, size_t length, const char *format, ...)
{
        assert(buffer != NULL && length > 0 && format != NULL);

        const size_t safe_length = length + 1;

        char *temp = calloc(1, safe_length * sizeof(char));
        if (temp == NULL) {
                fprintf(stderr, "ERROR: %s - (%s)\n",
                        error_message[ERROR_BUFFER_APPEND],
                        strerror(errno));

                return -1;
        }

        va_list args;
        va_start(args, format);

        int status = 0;
        status = vsnprintf(temp, safe_length, format, args);
        if (status < 0) {
                fprintf(stderr, "ERROR: %s - (vsnprintf() return: %d)\n",
                        error_message[ERROR_BUFFER_APPEND],
                        status);

                if (temp != NULL) {
                        free(temp);
                }

                return -1;
        }

        va_end(args);

        status = buffer_append(buffer, temp, length);
        if (status < 0) {
                if (temp != NULL) {
                        free(temp);
                }

                return -1;
        }

        free(temp);

        return 0;
}

char *
buffer_to_string(buffer *buffer)
{
        assert(buffer != NULL);

        const size_t bufferlen = buffer_length(buffer);

        if (bufferlen > 0) {
                char *string = calloc(1, bufferlen + 1);
                if (string == NULL) {
                        fprintf(stderr, "ERROR: %s - (%s)\n",
                                error_message[ERROR_BUFFER_TO_STRING],
                                strerror(errno));

                        return NULL;
                }

                memmove(string, buffer->data, bufferlen);

                return string;
        }

        return NULL;
}

void
buffer_free(buffer *buffer)
{
        if (buffer != NULL) {
                if (buffer->data != NULL) {
                        free(buffer->data);
                }

                buffer->length = 0;
                buffer->size = 0;

                free(buffer);
        }
}
