#include "buffer.h"
#include <stdlib.h>

void init_buffer(Buffer *buf, int size)
{
    buf->array = (Item*) malloc(size * sizeof(Item));
    buf->capacity = size;
    buf->in = 0;
    buf->out = 0;
}

void buffer_push(Buffer *buf, Item item)
{
    buf->array[buf->in] = item;
    buf->in = (buf->in + 1) % buf->capacity;
}

Item buffer_pop(Buffer *buf)
{
    Item item = buf->array[buf->out];
    buf->out = (buf->out + 1) % buf->capacity;
    return item;
}