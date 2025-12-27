#ifndef MY_SEMAPHORE
#define MY_SEMAPHORE
#include <semaphore.h>

void up(sem_t *sem);
void down(sem_t *sem);

#endif