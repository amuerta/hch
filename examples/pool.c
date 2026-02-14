#define POOL_USE_EXACT_TYPE_MATCH
#include "../src/pool.h"

// #define Pool hc_Pool

typedef struct EntityData {
    int i;
} EntityData;

typedef struct {
    EntityData data;
} Entity;

typedef hc_Pool(Entity) EntPool;

void pool_grow_if_needed(EntPool* pool) {
    if(pool_need_resize(pool)) {
        size_t newsz = pool_measure(pool->capacity * 2, hc_pool_type_size(pool));
        free(pool_grow_buffer(pool, calloc(newsz,1), newsz));
        pool->allocated_on_heap = true;
    }
}

// TODO: fully make this example work
int main(void) {
    printf("Running pool example\n");
    static char buffer[1<<10];
    EntPool pool = {0};
    pool_from_heap(&pool, 2);
    
    pool_index ids[20] = {0};
    (void)ids;
    (void)buffer;

    for(int i = 0; i < 20; i++) {
        pool_grow_if_needed(&pool);
        Entity e = { .data = {.i = i }};
        ids[i] = pool_insert(&pool, &e);
    }

    // Entity *e;
    // for(int i = 0; pool_next(&pool, &e); i++) {
    //     if(e) printf("e%02i : %i\n", i, e->data.i);
    //     e = 0;
    // }

    size_t m = pool_measure(2, sizeof(Entity));
    free(pool_resize_buffer(&pool, calloc(m,1), m));

    // printf("After resize: \n");
    //
    // for(int i = 0; pool_next(&pool, &e); i++) {
    //     if(e) printf("e%02i : %i\n", i, e->data.i);
    //     e = 0;
    // }

    free(pool.items);
}
