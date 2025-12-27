#include "produce_making.h"
#include <stdio.h>

Item produce_dough(int maker_id, FILE *file)
{
    static int global_counter = 0;

    Item item;
    item.id = global_counter++;
    item.type = 'D';

    printf("Producer %d: produced dough nr %d, %c\n", maker_id, item.id, item.type);
    fprintf(file, "Producer %d: produced dough nr %d, %c\n", maker_id, item.id, item.type);
    return item;
}


Item produce_cabbage(int maker_id, FILE *file)
{
    static int global_counter = 0;

    Item item;
    item.id = global_counter++;
    item.type = 'C';

    printf("Producer %d: produced cabbage nr %d, %c\n", maker_id, item.id, item.type);
    fprintf(file, "Producer %d: produced cabbage nr %d, %c\n", maker_id, item.id, item.type);
    return item;
}


Item produce_meat(int maker_id, FILE *file)
{
    static int global_counter = 0;

    Item item;
    item.id = global_counter++;
    item.type = 'M';

    printf("Producer %d: produced meat nr %d, %c\n", maker_id, item.id, item.type);
    fprintf(file, "Producer %d: produced meat nr %d, %c\n", maker_id, item.id, item.type);
    return item;
}


Item produce_cheese(int maker_id, FILE *file)
{
    static int global_counter = 0;

    Item item;
    item.id = global_counter++;
    item.type = 'S';

    printf("Producer %d: produced cheese nr %d, %c\n", maker_id, item.id, item.type);
    fprintf(file, "Producer %d: produced cheese nr %d, %c\n", maker_id, item.id, item.type);
    return item;
}
