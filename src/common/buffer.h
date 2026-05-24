#ifndef COMMON_BUFFER_H
#define COMMON_BUFFER_H
#include <stddef.h>

typedef struct {
    char *data;
    size_t size;
    size_t capacity;
} ByteBuffer;

void buffer_init(ByteBuffer *buf, size_t initial_capacity);
void buffer_append(ByteBuffer *buf, const char *data, size_t len);
int buffer_extract_line(ByteBuffer *buf, char **line_out);
void buffer_free(ByteBuffer *buf);

#endif
