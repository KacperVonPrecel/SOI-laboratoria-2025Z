#include <pthread.h>
#include "produce_making.h"

typedef struct {
    Item *array;
    int capacity;
    int count;
    int in;
    int out;

    pthread_mutex_t lock;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
} BufferMonitor;

void monitor_init(BufferMonitor *buf, int size);
void monitor_put(BufferMonitor *buf, Item item, FILE *file);
Item monitor_get(BufferMonitor *buf);
void monitor_destroy(BufferMonitor *buf);
