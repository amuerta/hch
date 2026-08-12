#define HC_POOL_STRIP_PREFIX
#include <pool.h>

// #define Pool hc_Pool

typedef struct {
    struct {int i;} data;
} Entity;

typedef hc_Pool(Entity) EntPool;

void pool_grow_if_needed(EntPool* pool) {
    if(pool_need_resize(pool)) {
        size_t newsz = pool_measure(*pool, pool->capacity * 2);
        free(pool_grow_buffer(pool, calloc(newsz,1), newsz));
        pool->allocated_on_heap = true;
    }
}


// int main(void) {
//     static char buffer[1<<10];
//     Pool(int) pool = {0};
//     pool_from_buffer(&pool, buffer, sizeof(buffer));
//
//     for(int i = 0; i < 10; i++) {
//         pool_insert(&pool, &i);
//     }
//
//     printf("cap:    %lu\n", pool.capacity);
//     printf("max_c:  %lu\n", pool.max_count);
//
//     for(size_t i = 0; i < pool.max_count; i++) {
//         // if(hc_pool_is_slot_used(pool.items[i])) 
//             printf("e%02lu : %i\n", i, hc_pool_get(pool, i));
//     }
// }

// TODO: fully make this example work



#define Pool hc_Pool
#define pool_release hc_pool_release_item

int main(void) {
    EntPool pool = {0};
    hc_pool_index ids[20] = {0};
   
    pool_from_heap(&pool, 2);

    for(int i = 0; i < 20; i++) {
        pool_grow_if_needed(&pool);
        Entity e = { .data = {.i = i }};
        ids[i] = pool_insert(&pool, &e);
    }

    pool_release(&pool, ids[3]);
    pool_release(&pool, ids[4]);
    pool_release(&pool, ids[5]);

    for(int i = 20; i < 20+3; i++) {
        pool_insert(&pool, &i);
    }

    // __asm__("int3");
    hc_pool_foreach(pool, i) {
        printf("e%02lu : %i\n", i, hc_pool_get(pool, i).data.i);
    }

    size_t m = pool_measure(pool, 2);
    free(pool_resize_buffer(&pool, calloc(m,1), m));

    hc_pool_foreach(pool, i) {
        printf("e%02lu : %i\n", i, hc_pool_get(pool, i).data.i);
    }

    free(pool.items);
}
