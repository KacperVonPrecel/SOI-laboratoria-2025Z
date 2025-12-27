#include "monitor.h"

void monitor_init(BufferMonitor *buf, int size)
{
    buf->array = (Item*) malloc(size * sizeof(Item));
    buf->capacity = size;
    buf->count = 0;
    buf->in = 0;
    buf->out = 0;

    pthread_mutex_init(&buf->lock, NULL);
    pthread_cond_init(&buf->not_full, NULL);
    pthread_cond_init(&buf->not_empty, NULL);
}

void monitor_put(BufferMonitor *buf, Item item)
{
    pthread_mutex_lock(&buf->lock);

    while(buf->count == buf->capacity)
    {
        pthread_cond_wait(&buf->not_full, &buf->lock);
    }

    buf->array[buf->in] = item;
    buf->in = (buf->in + 1) % buf->capacity;
    buf->count++;

    pthread_cond_signal(&buf->not_empty);

    pthread_mutex_ulock(&buf->lock);
}

Item monitor_get(BufferMonitor *buf)
{
    pthread_mutex_lock(&buf->lock);

    while (buf->count == 0)
    {
        pthread_cond_wait(&buf->not_empty, &buf->lock);
    }

    Item item = buf->array[buf->out];
    buf->out = (buf->out + 1) % buf->capacity;
    buf->count--;

    pthread_cond_signal(&buf->not_full);

    pthread_mutex_unlock(&buf->lock);

    return item;
}

void monitor_destroy(BufferMonitor *buf)
{
    free(buf->array);
    pthread_mutex_destroy(&buf->lock);
    pthread_cond_destroy(&buf->not_empty);
    pthread_cond_destroy(&buf->not_full);
}