#include "my_semaphore.h"

void down(sem_t *sem)
{
    sem_wait(sem);
}

void up(sem_t *sem)
{
    sem_post(sem);
}
