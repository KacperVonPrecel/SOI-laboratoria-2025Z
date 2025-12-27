#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "my_semaphore.h"
#include "produce_making.h"
#include "buffer.h"

Buffer *buffer_dough;
Buffer *buffer_meat;
Buffer *buffer_cabbage;
Buffer *buffer_cheese;
int N;

sem_t mutex_dough, full_dough, empty_dough;
sem_t mutex_meat, full_meat, empty_meat;
sem_t mutex_cabbage, full_cabbage, empty_cabbage;
sem_t mutex_cheese, full_cheese, empty_cheese;

static int DOUGH_PRODUCER_ID = 1;
static int MEAT_PRODUCER_ID = 2;
static int CABBAGE_PRODUCER_ID = 3;
static int CHEESE_PRODUCER_ID = 4;

void make_pierogi(Item dough, Item filling, FILE *file);

void producer_dough(FILE *file) {
    while(1)
    {
        Item item = produce_dough(DOUGH_PRODUCER_ID, file);

        down(&empty_dough);
        down(&mutex_dough);

        buffer_push(buffer_dough, item);
        printf("[IN BUFFOR] Dough nr %d put in buffor\n", item.id);
        fprintf(file, "[IN BUFFOR] Dough nr %d put in buffor\n", item.id);

        up(&mutex_dough);
        up(&full_dough);
        sleep(1);
    }
}

void producer_meat(FILE *file) {
    while(1)
    {
        Item item = produce_meat(MEAT_PRODUCER_ID, file);

        down(&empty_meat);
        down(&mutex_meat);

        buffer_push(buffer_meat, item);
        printf("[IN BUFFOR] Meat nr %d put in buffor\n", item.id);
        fprintf(file, "[IN BUFFOR] Meat nr %d put in buffor\n", item.id);

        up(&mutex_meat);
        up(&full_meat);
        sleep(1);
    }
}

void producer_cabbage(FILE *file) {
    while(1)
    {
        Item item = produce_cabbage(CABBAGE_PRODUCER_ID, file);

        down(&empty_cabbage);
        down(&mutex_cabbage);

        buffer_push(buffer_cabbage, item);
        printf("[IN BUFFOR] Cabbage nr %d put in buffor\n", item.id);
        fprintf(file, "[IN BUFFOR] Cabbage nr %d put in buffor\n", item.id);

        up(&mutex_cabbage);
        up(&full_cabbage);
        sleep(1);
    }
}

void producer_cheese(FILE *file) {
    while(1)
    {
        Item item = produce_cheese(CHEESE_PRODUCER_ID, file);

        down(&empty_cheese);
        down(&mutex_cheese);

        buffer_push(buffer_cheese, item);
        printf("[IN BUFFOR] Cheese nr %d put in buffor\n", item.id);
        fprintf(file, "[IN BUFFOR] Cheese nr %d put in buffor\n", item.id);

        up(&mutex_cheese);
        up(&full_cheese);
        sleep(1);
    }
}


void consumer_pierogi_meat(FILE *file)
{
    while (1)
    {
        down(&full_meat);
        down(&mutex_meat);

        Item meat = buffer_pop(buffer_meat);

        up(&mutex_meat);
        up(&empty_meat);

        down(&full_dough);
        down(&mutex_dough);

        Item dough = buffer_pop(buffer_dough);

        up(&mutex_dough);
        up(&empty_dough);

        make_pierogi(dough, meat, file);
    }
}

void consumer_pierogi_cheese(FILE *file)
{
    while (1)
    {
        down(&full_cheese);
        down(&mutex_cheese);

        Item cheese = buffer_pop(buffer_cheese);

        up(&mutex_cheese);
        up(&empty_cheese);

        down(&full_dough);
        down(&mutex_dough);

        Item dough = buffer_pop(buffer_dough);

        up(&mutex_dough);
        up(&empty_dough);

        make_pierogi(dough, cheese, file);
    }
}

void consumer_pierogi_cabbage(FILE *file)
{
    while (1)
    {
        down(&full_cabbage);
        down(&mutex_cabbage);

        Item cabbage = buffer_pop(buffer_cabbage);

        up(&mutex_cabbage);
        up(&empty_cabbage);

        down(&full_dough);
        down(&mutex_dough);

        Item dough = buffer_pop(buffer_dough);

        up(&mutex_dough);
        up(&empty_dough);

        make_pierogi(dough, cabbage, file);
    }
}


int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage: %s <buffor_size> <file_name.txt>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[2], "w");

    if (file == NULL)
    {
        printf("Error! Unable to open a save file!\n");
        return 1;
    }

    N = atoi(argv[1]);
    printf("Simulation starts. Buffors sizes: %d\n", N);
    fprintf(file, "Simulation starts. Buffors sizes: %d\n", N);

    buffer_dough = malloc(sizeof(Buffer));
    buffer_meat = malloc(sizeof(Buffer));
    buffer_cabbage = malloc(sizeof(Buffer));
    buffer_cheese = malloc(sizeof(Buffer));

    init_buffer(buffer_dough, N);
    init_buffer(buffer_meat, N);
    init_buffer(buffer_cabbage, N);
    init_buffer(buffer_cheese, N);

    sem_init(&mutex_dough, 0, 1);
    sem_init(&full_dough, 0, 0);
    sem_init(&empty_dough, 0, N);

    sem_init(&mutex_meat, 0, 1);
    sem_init(&full_meat, 0, 0);
    sem_init(&empty_meat, 0, N);

    sem_init(&mutex_cabbage, 0, 1);
    sem_init(&full_cabbage, 0, 0);
    sem_init(&empty_cabbage, 0, N);

    sem_init(&mutex_cheese, 0, 1);
    sem_init(&full_cheese, 0, 0);
    sem_init(&empty_cheese, 0, N);

    pthread_t prod_table[4];
    pthread_t cons_table[3];

    printf("All threads running. We work for 10 seconds..\n");
    fprintf(file, "All threads running. We work for 10 seconds..\n");

    pthread_create(&cons_table[0], NULL, (void*)consumer_pierogi_meat, file);
    pthread_create(&cons_table[1], NULL, (void*)consumer_pierogi_cheese, file);
    pthread_create(&cons_table[2], NULL, (void*)consumer_pierogi_cabbage, file);

    pthread_create(&prod_table[0], NULL, (void*)producer_dough, file);
    pthread_create(&prod_table[1], NULL, (void*)producer_meat, file);
    pthread_create(&prod_table[2], NULL, (void*)producer_cabbage, file);
    pthread_create(&prod_table[3], NULL, (void*)producer_cheese, file);

    sleep(10);

    printf("\nEnd of simulation! Closing threads.\n");
    fprintf(file, "\nEnd of simulation! Closing threads.\n");
    sem_destroy(&mutex_dough);
    sem_destroy(&full_dough);
    sem_destroy(&empty_dough);

    sem_destroy(&mutex_meat);
    sem_destroy(&full_meat);
    sem_destroy(&empty_meat);

    sem_destroy(&mutex_cabbage);
    sem_destroy(&full_cabbage);
    sem_destroy(&empty_cabbage);

    sem_destroy(&mutex_cheese);
    sem_destroy(&full_cheese);
    sem_destroy(&empty_cheese);

    free(buffer_dough);
    free(buffer_meat);
    free(buffer_cabbage);
    free(buffer_cheese);

    fclose(file);

    exit(0);
}

void make_pierogi(Item dough, Item filling, FILE *file)
{
    printf("Making pierog with %c; (dough id: %d, filling id %d)\n", filling.type, dough.id, filling.id);
    fprintf(file, "Making pierog with %c; (dough id: %d, filling id %d)\n", filling.type, dough.id, filling.id);
    sleep(2);
}