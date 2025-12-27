#ifndef BUFFER
#define BUFFER
#include "produce_making.h"

typedef struct {
    Item *array;
    int capacity;
    int in;
    int out;
} Buffer;

void init_buffer(Buffer *buf, int size);
void buffer_push(Buffer *buf, Item item);
Item buffer_pop(Buffer *buf);

#endif