#include "chess.h"

#define INITIAL_CAPACITY 25

typedef struct ArrayList {
    struct state *arr;
    int capacity;
    int size;
} ArrayList;

ArrayList *createArrayList();
void add(ArrayList *list, struct state element);
struct state get(ArrayList *list, int index);
struct state pop(ArrayList *list);
int size(ArrayList *list);
void destroyArrayList(ArrayList *list);
