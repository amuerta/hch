#ifndef __EOBJECT_POOL_H
#define __EOBJECT_POOL_H

/*  POOL DATA STRUCTURE
 *
 *  Pool keeps location of objects in memory consistent and you access them via pool_index instead of pointer, even tho using a pointer
 *  is a valid method too. Benefit of Pool is that when working with large number of Entities, insertion and deletion operations take `O(1)`.
 *  
 *  NOTES:
 *      + resizing `Pool` doesn't update `free_list` as this operation can be confusing or ambiguous depending on your way of use. So when you downsize
 *          your pool - migrating is done manually.
 *      + API of pool is designed to follow my "allocator free" design idea, thus you work with buffers that are fed into pool, instead of giving/defining allocator functions. 
 *          Link to my post about this idea: <TBD>
 * */

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#define hc_Pool(T) struct {                                 \
    struct { char infomask; T item;} *items;                \
    pool_index      *free_list;                             \
    size_t          count, capacity, max_count, free_count; \
    bool            allocated_on_heap;                      \
}

#define HC_POOL_INVALID_INDEX ((size_t)(-1))
typedef size_t pool_index;

// PoolSlot is a generic container to fit your data
typedef struct {
    unsigned char   infomask; // bool
    unsigned char   data[];
} hc_PoolSlotBase;

typedef struct {
    hc_PoolSlotBase *items;
    pool_index      *free_list;
    size_t          count, capacity, 
                    max_count, free_count;
    bool            allocated_on_heap;
} hc_PoolBase;

enum {
    HC_POOL_SLOT_IS_EMPTY = 0,
    HC_POOL_SLOT_IS_USED  = 1,
};


#define hc_pool_type_size(P) sizeof(((P)->items[0].item))

#define pool_get(P, i)\
    ((P)->items[pool_assert_index(i)].item)

#define pool_insert(P, I)\
    pool_insert_ex(P, hc_pool_type_size(P), I, sizeof(*(I)))

#define pool_from_heap(P, count) \
    pool_from_heap_ex(P, hc_pool_type_size(P), count)

#define pool_from_buffer(P, buf, size) \
    pool_from_buffer_ex(P, hc_pool_type_size(P), buf, size)

#define pool_resize_buffer(P, new_buf, size)\
    pool_resize_buffer_ex(P, hc_pool_type_size(P), new_buf, size)

#define pool_grow_buffer(P, new_buf, size)\
    pool_grow_buffer_ex(P, hc_pool_type_size(P), new_buf, size)


#define pool_max(A,B) (((A) > (B)) ? (A) : (B))
#define pool_min(A,B) (((A) < (B)) ? (A) : (B))


#ifndef POOL_HEADER_ONLY

// creates Pool that fits into given buffer, lazy way to create a pool out of
// static buffer, for testing purpouses.

void pool_from_buffer_ex(void* p, size_t typesize, void* buffer, size_t buffer_size) {
    assert(p && "expected valid pointer too pool");
    hc_PoolBase *pool = p;
    
    size_t indexsize        = sizeof(pool->free_list[0]);
    size_t full_typesize    = sizeof(pool->items[0]) + typesize;
    // float  split_ratio      = pool_max(full_typesize, indexsize) / pool_min(full_typesize, indexsize);
    size_t count_items_fit  = buffer_size / (full_typesize + indexsize);
    // split buffer between free_list and items:
    // [{              }xxx{     }]
    //  ^ items         |  ^ free_list
    //                  +- padding (leftover bytes from fitting process)
    void* items     =  buffer;
    void* free_list = (buffer + (buffer_size - indexsize*count_items_fit));

    pool->items      = items;
    pool->free_list  = free_list;
    pool->capacity   = count_items_fit;
    pool->items      = buffer;
    pool->allocated_on_heap = false;
}


void pool_from_heap_ex(void* p, size_t typesize, size_t count) {
    assert(p && "expected valid pointer too pool");
    assert(count && typesize);
    hc_PoolBase *pool = p;

    size_t item_size =      count * (sizeof(pool->items[0]));
    size_t free_list_size = count * (sizeof(pool->free_list[0]));
    assert(item_size && free_list_size);

    void* data = calloc(count, item_size + free_list_size); 
    pool_from_buffer_ex(p, typesize, data, item_size+free_list_size);
    pool->allocated_on_heap = true;
}

size_t pool_measure(size_t count, size_t typesize);

size_t pool_measure(size_t count, size_t typesize) {
    const hc_PoolBase       pool;
    const hc_PoolSlotBase   slot;
    return count*(typesize + sizeof(slot)) 
         + count*sizeof(pool.free_list[0]);
}

void* pool_resize_buffer_ex(void* p, size_t typesize, void* new_buffer, size_t new_buffer_size) {
    hc_PoolBase *pool   = p;
    hc_PoolBase copy    = *pool;
    void* old_ptr       = pool->items;

    void* old_items     = copy.items;
    void* old_free_list = copy.free_list;

    size_t old_items_size = 0,
           old_free_list_size = 0,
           new_items_size = 0,
           new_free_list_size = 0;

    old_items_size       = (typesize + sizeof(pool->items[0]))*pool->capacity;
    old_free_list_size   = sizeof(pool->free_list[0])*pool->capacity;
    pool_from_buffer_ex(pool, typesize, new_buffer, new_buffer_size);
    new_items_size       = (typesize + sizeof(pool->items[0]))*pool->capacity;
    new_free_list_size   = sizeof(pool->free_list[0])*pool->capacity;

    size_t items_size     = pool_min(old_items_size, new_items_size);
    size_t free_list_size = pool_min(old_free_list_size, new_free_list_size);

    if(old_items_size > new_items_size) {
        pool->max_count = pool->capacity;
        pool->count = pool->capacity;
    } else 
        pool->count = copy.count;

    memcpy(pool->items, old_items, items_size);
    memcpy(pool->free_list, old_free_list, free_list_size);

    return old_ptr;
}

void* pool_grow_buffer_ex(void* p, size_t typesize, void* new_buffer, size_t new_buffer_size) {
    hc_PoolBase *pool = p;
    assert(pool_measure(pool->capacity, typesize) < new_buffer_size && "`pool_grow_buffer` was given smaller buffer size then before.");
    return pool_resize_buffer_ex(p, typesize, new_buffer, new_buffer_size);
}


inline void pool_heap_free(void* p) {
    hc_PoolBase *pool = p;
    assert(pool->allocated_on_heap);
    if(pool->items) free(pool->items);
}

// TODO: slightly improve performace by stopping after seeing the last item count wise.
bool pool_ref_next(void* pool, size_t typesize, void** out) {
    (void) typesize;
    hc_PoolBase *p = pool;
    static pool_index id = 0;
    if(id >= p->capacity) {
        id = 0;
        return false;
    }
    void* ptr = p->items + id * sizeof(p->items[0]);
    if(ptr) *out = ptr;
    id++;
    return true;
}

bool pool_check_index(pool_index i) {
    return i != HC_POOL_INVALID_INDEX;
}

void pool_assert_index(pool_index i) {
    assert(pool_check_index(i) && "Attempt to access unset item");
}

    
bool pool_has_space(void* pool) {
    hc_PoolBase *p = pool;
    return p->count < p->capacity;
}

bool pool_need_resize(void* pool) {
    hc_PoolBase *p = pool;
    return p->count >= p->capacity;
}



pool_index pool_reserve(void* pool, size_t typesize) {
    hc_PoolBase *p = pool;
    assert(p->items && p->free_list && p->capacity && typesize);
    pool_index index = (size_t)(-1);
    if (p->free_count > 0) {
        index = p->free_list[p->free_count-1];
        p->free_count--;
    } else {
        index = p->count;
    }
    assert(index != HC_POOL_INVALID_INDEX   && "Failed to reserve entity");
    assert(p->count < p->capacity           && "Attempt to buffer overflow");

    size_t fullsize = sizeof(p->items[0]) + typesize ;
    hc_PoolSlotBase* item = p->items + fullsize*index;
    item->infomask = HC_POOL_SLOT_IS_USED;
    p->count++;
    
    if (p->count > p->max_count) {
        p->max_count = p->count;
    }
    return index;
}

void pool_release(void* pool, size_t typesize, pool_index i) {
    hc_PoolBase *p = pool;
    assert(p->items && p->free_list && p->capacity && typesize);
    if (p->count==0)
        return;
    assert(i < p->capacity && "Attempt to access Out of Bounds");

    size_t fullsize = sizeof(p->items[0]) + typesize ;
    hc_PoolSlotBase* item = (p->items + i*fullsize);

    if (!item->infomask) return;

    p->free_list[p->free_count] = i;
    p->free_count++;

    item->infomask = false;
    p->count--;
}


pool_index pool_insert_ex(void* pool, size_t typesize, void* item, size_t size) {
    hc_PoolBase *p = pool;
    assert(typesize == size);
    pool_index i = pool_reserve(pool, typesize);
    size_t fullsize = sizeof(p->items[0]) + typesize ;
    void* it = p->items + fullsize*i;
    memcpy(it, item, size);
    return i;
}

#endif // POOL_HEADER_ONLY

#endif//__EOBJECT_POOL_H
