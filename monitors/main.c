#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "produce_making.h"
#include "monitor.h"

BufferMonitor *monitor_dough;
BufferMonitor *monitor_meat;
BufferMonitor *monitor_cabbage;
BufferMonitor *monitor_cheese;
int N;

static int DOUGH_PRODUCER_ID = 1;
static int MEAT_PRODUCER_ID = 2;
static int CABBAGE_PRODUCER_ID = 3;
static int CHEESE_PRODUCER_ID = 4;

void make_pierogi(Item dough, Item filling, FILE *file);

void producer_dough(FILE *file) {
    while(1)
    {
        Item item = produce_dough(DOUGH_PRODUCER_ID, file);
        monitor_put(monitor_dough, item, file);
        sleep(1);
    }
}

void producer_meat(FILE *file) {
    while(1)
    {
        Item item = produce_meat(MEAT_PRODUCER_ID, file);
        monitor_put(monitor_meat, item, file);
        sleep(1);
    }
}

void producer_cabbage(FILE *file) {
    while(1)
    {
        Item item = produce_cabbage(CABBAGE_PRODUCER_ID, file);
        monitor_put(monitor_cabbage, item, file);
        sleep(1);
    }
}

void producer_cheese(FILE *file) {
    while(1)
    {
        Item item = produce_cheese(CHEESE_PRODUCER_ID, file);
        monitor_put(monitor_cheese, item, file);
        sleep(1);
    }
}


void consumer_pierogi_meat(FILE *file)
{
    while (1)
    {
        Item meat = monitor_get(monitor_meat);
        Item dough = monitor_get(monitor_dough);
        make_pierogi(dough, meat, file);
    }
}

void consumer_pierogi_cheese(FILE *file)
{
    while (1)
    {
        Item cheese = monitor_get(monitor_cheese);
        Item dough = monitor_get(monitor_dough);
        make_pierogi(dough, cheese, file);
    }
}

void consumer_pierogi_cabbage(FILE *file)
{
    while (1)
    {
        Item cabbage = monitor_get(monitor_cabbage);
        Item dough = monitor_get(monitor_dough);
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

    monitor_dough = malloc(sizeof(BufferMonitor));
    monitor_meat = malloc(sizeof(BufferMonitor));
    monitor_cabbage = malloc(sizeof(BufferMonitor));
    monitor_cheese = malloc(sizeof(BufferMonitor));

    monitor_init(monitor_dough, N);
    monitor_init(monitor_meat, N);
    monitor_init(monitor_cabbage, N);
    monitor_init(monitor_cheese, N);

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

    fflush(file);
    exit(0);
}

void make_pierogi(Item dough, Item filling, FILE *file)
{
    printf("Making pierog with %c; (dough id: %d, filling id %d)\n", filling.type, dough.id, filling.id);
    fprintf(file, "Making pierog with %c; (dough id: %d, filling id %d)\n", filling.type, dough.id, filling.id);
    sleep(2);
}