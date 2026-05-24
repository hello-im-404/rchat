#include "buffer.h"
#include <stdlib.h>
#include <string.h>

void buffer_init(ByteBuffer *buf, size_t initial_capacity) {
    buf->data = (char *)malloc(initial_capacity);
    buf->size = 0;
    buf->capacity = initial_capacity;
}

void buffer_append(ByteBuffer *buf, const char *data, size_t len) {
    if (buf->size + len > buf->capacity) {
        while (buf->size + len > buf->capacity) {
            buf->capacity *= 2;
        }
        buf->data = (char *)realloc(buf->data, buf->capacity);
    }
    memcpy(buf->data + buf->size, data, len);
    buf->size += len;
}

int buffer_extract_line(ByteBuffer *buf, char **line_out) {
    for (size_t i = 0; i < buf->size; i++) {
        if (buf->data[i] == '\n') {
            size_t line_len = i;
            *line_out = (char *)malloc(line_len + 1);
            memcpy(*line_out, buf->data, line_len);
            (*line_out)[line_len] = '\0';
            
            if (line_len > 0 && (*line_out)[line_len - 1] == '\r') {
                (*line_out)[line_len - 1] = '\0';
            }

            size_t remaining = buf->size - (i + 1);
            memmove(buf->data, buf->data + i + 1, remaining);
            buf->size = remaining;
            return 1;
        }
    }
    return 0;
}

void buffer_free(ByteBuffer *buf) {
    if (buf->data) {
        free(buf->data);
        buf->data = NULL;
    }
    buf->size = 0;
    buf->capacity = 0;
}
