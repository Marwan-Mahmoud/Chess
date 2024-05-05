#include <stdio.h>
#include <stdlib.h>
#include "arraylist.h"

ArrayList *createArrayList()
{
    ArrayList *list = (ArrayList *)malloc(sizeof(ArrayList));
    list->capacity = INITIAL_CAPACITY;
    list->size = 0;
    list->arr = (struct state *)malloc(list->capacity * sizeof(struct state));
    return list;
}

void add(ArrayList *list, struct state element)
{
    if (list->size == list->capacity)
    {
        list->capacity *= 2;
        struct state *newArr = (struct state *)realloc(list->arr, list->capacity * sizeof(struct state));
        list->arr = newArr;
    }
    list->arr[list->size++] = element;
}

struct state get(ArrayList *list, int index)
{
    return list->arr[index];
}

struct state pop(ArrayList *list)
{
    if (list->size == 1)
        return list->arr[0];
    else {
        list->size--;
        return list->arr[list->size - 1];
    }
}

void destroyArrayList(ArrayList *list)
{
    free(list->arr);
    free(list);
}
