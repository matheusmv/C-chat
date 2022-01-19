#include "buffer.h"
#include "clogger.h"

buffer *
new_buffer(uint size)
{
        buffer *buf = malloc(sizeof(struct buffer));
        if (buf == NULL) {
                LOG_ERROR("malloc() error. %s", strerror(errno));
                return NULL;
        }

        char *data = calloc(1, size * sizeof(char));
        if (data == NULL) {
                LOG_ERROR("calloc() error. %s", strerror(errno));
                free(buf);
                return NULL;
        }

        *buf = (struct buffer) {
                .data = data,
                .length = 0,
                .size = size
        };

        return buf;
}

int
buffer_length(buffer *buffer)
{
        if (buffer != NULL)
                return buffer->length;

        return -1;
}

int
buffer_size(buffer *buffer)
{
        if (buffer != NULL)
                return buffer->size;

        return -1;
}

static int
buffer_has_storage_available(buffer *buffer, uint length)
{
        const uint available_storage = buffer->size - buffer->length;

        return length <= available_storage;
}

static int
increase_storage_capacity(buffer *buffer, uint size)
{
        if (buffer != NULL) {
                const uint buffer_size = buffer->size;
                const uint new_size = buffer_size < size ? (size * 2) : (buffer_size * 2);

                char *new_sdata = realloc(buffer->data, new_size * sizeof(char));
                if (new_sdata == NULL) {
                        LOG_ERROR("realloc() error. %s", strerror(errno));
                        return -1;
                }

                *buffer = (struct buffer) {
                        .data = new_sdata,
                        .size = new_size,
                        .length = buffer->length
                };

                return 0;
        }

        return -1;
}

int
buffer_append(buffer *buffer, const char *src, uint src_length)
{
        if (buffer != NULL) {
                if (!buffer_has_storage_available(buffer, src_length + 1)) {
                        if (increase_storage_capacity(buffer, src_length + 1) < 0) {
                                LOG_ERROR("increase_storage_capacity() error.");
                                return -1;
                        }
                }

                strncat(buffer->data, src, src_length);
                buffer->length += src_length;

                return 0;
        }

        return -1;
}

int
buffer_appendf(buffer *buffer, const char *format, ...)
{
        va_list args;
        va_start(args, format);

        char *temp = NULL;
        int length = vasprintf(&temp, format, args);
        if (length < 0) {
                LOG_ERROR("vasprintf() error.");
                if (temp != NULL)
                        free(temp);

                return -1;
        }

        va_end(args);

        int status = 0;
        status = buffer_append(buffer, temp, length);
        if (status < 0) {
                LOG_ERROR("buffer_append() error.");
                if (temp != NULL)
                        free(temp);

                return -1;
        }

        free(temp);

        return 0;
}

int
buffer_nappendf(buffer *buffer, size_t length, const char *format, ...)
{
        char *temp = calloc(1, (length + 1) * sizeof(char));
        if (temp == NULL) {
                LOG_ERROR("calloc() error. %s", strerror(errno));
                return -1;
        }

        va_list args;
        va_start(args, format);

        int status = 0;
        status = vsnprintf(temp, (length + 1), format, args);
        if (status < 0) {
                LOG_ERROR("vsnprintf() error.");
                if (temp != NULL)
                        free(temp);

                return -1;
        }

        va_end(args);

        status = buffer_append(buffer, temp, length);
        if (status < 0) {
                LOG_ERROR("buffer_append() error.");
                if (temp != NULL)
                        free(temp);

                return -1;
        }

        free(temp);

        return 0;
}

char *
buffer_to_string(buffer *buffer)
{
        if (buffer != NULL && buffer->length > 0) {
                char *string = calloc(1, buffer->length + 1);
                if (string == NULL) {
                        LOG_ERROR("calloc() error. %s", strerror(errno));
                        return NULL;
                }

                memmove(string, buffer->data, buffer_length(buffer));

                return string;
        }

        return NULL;
}

void
buffer_free(buffer *buffer)
{
        if (buffer != NULL) {
                if (buffer->data != NULL)
                        free(buffer->data);

                buffer->length = 0;
                buffer->size = 0;

                free(buffer);
        }
}
