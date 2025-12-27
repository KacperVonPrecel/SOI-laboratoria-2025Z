#ifndef PRODUCE_MAKING
#define PRODUCE_MAKING
#include <stdio.h>

typedef struct
{
    int id;
    char type;
} Item;

Item produce_dough(int maker_id, FILE *file);
Item produce_meat(int maker_id,FILE *file);
Item produce_cabbage(int maker_id, FILE *file);
Item produce_cheese(int maker_id, FILE *file);

#endif