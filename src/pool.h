#ifndef __HC_POOL_H
#define __HC_POOL_H

/*  POOL DATA STRUCTURE
 *
 *  Pool keeps location of objects in memory consistent and you access them via hc_pool_get instead of trying to use pointers,
 *  Benefit of Pool is that when working with large number of Entities, insertion and deletion operations take `O(1)`.
 *  
 *  NOTES:
 *      + resizing `Pool` doesn't update `free_list` as this operation can be confusing or ambiguous depending on your way of use. So when you downsize
 *          your pool - migrating is done manually.
 *      + API of pool is designed to follow my "allocator free" design idea, you work with buffers that are fed into data strucutre, 
 *          instead of giving function pointers to allocator functions. 
 *          Link to my post about this idea: <TBD>
 * */

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#define hc_Pool(T) struct {                                 \
    struct          { int infomask; T item;} *items;        \
    hc_pool_index   *free_list;                             \
    size_t          count, capacity, max_count, free_count; \
    bool            allocated_on_heap;                      \
}

#define HC_POOL_INVALID_INDEX ((size_t)(-1))
typedef size_t hc_pool_index;
typedef int    hc_pool_bitmask8;

// PoolSlot is a generic container to fit your data
typedef struct {
    int             infomask; // bool
    char            data[];   
} hc_PoolSlotBase;

typedef struct {
    void            *items;
    hc_pool_index   *free_list;
    size_t          count, capacity, 
                    max_count, free_count;
    bool            allocated_on_heap;
} hc_PoolBase;

enum {
    HC_POOL_SLOT_IS_EMPTY = 0,
    HC_POOL_SLOT_IS_USED  = 1,
};


#define hc_pool_slot_size(P)        sizeof((P).items[0])
#define hc_pool_type_size(P)        sizeof((P).items[0].item)
#define hc_pool_end(P)              (P.max_count)
#define hc_pool_is_slot_used(S)     (S.infomask & HC_POOL_SLOT_IS_USED)
#define hc_pool_get(P, i)           ((P).items[hc_pool_assert_index(i)].item)
#define hc_pool_measure(T, n)       hc_pool_measure_ex(sizeof((T).items[0]), n)

#define hc_pool_reserve(P)\
    hc_pool_reserve_ex(P,\
            hc_pool_slot_size(*(P)),\
            hc_pool_type_size(*(P)))

#define hc_pool_release(P, I)\
    hc_pool_release_ex(P,\
            hc_pool_slot_size(*(P)),\
            hc_pool_type_size(*(P)),\
            I)

#define hc_pool_insert(P, I)\
    hc_pool_insert_ex(P,\
            hc_pool_slot_size(*(P)),\
            hc_pool_type_size(*(P)),\
            I, sizeof(*(I)))

#define hc_pool_from_heap(P, count) \
    hc_pool_from_heap_ex(P,\
            hc_pool_slot_size(*(P)),\
            hc_pool_type_size(*(P)),\
            count)

#define hc_pool_from_buffer(P, buf, size) \
    hc_pool_from_buffer_ex(P,\
            hc_pool_slot_size(*(P)),\
            hc_pool_type_size(*(P)),\
            buf, size)

#define hc_pool_resize_buffer(P, new_buf, size)\
    hc_pool_resize_buffer_ex(P,\
            hc_pool_slot_size(*(P)),\
            hc_pool_type_size(*(P)),\
            new_buf, size)

#define hc_pool_grow_buffer(P, new_buf, size)\
    hc_pool_grow_buffer_ex(P, \
            hc_pool_slot_size(*(P)),\
            hc_pool_type_size(*(P)),\
            new_buf, size)

#define hc_pool_foreach(P, I) \
    for(size_t I = 0; i < hc_pool_end(P); I++)\
    if(hc_pool_is_slot_used((P).items[I]))

#define hc_pool_max(A,B) (((A) > (B)) ? (A) : (B))
#define hc_pool_min(A,B) (((A) < (B)) ? (A) : (B))


size_t          hc_pool_measure_ex(size_t slotsize, size_t count);
bool            hc_pool_is_index_ok(hc_pool_index i);
size_t          hc_pool_assert_index(hc_pool_index i);
bool            hc_pool_has_space(void* pool);
bool            hc_pool_need_resize(void* pool);

void            hc_pool_from_buffer_ex(void* p, size_t slotsize, size_t typesize, void* buffer, size_t buffer_size) ;
void            hc_pool_from_heap_ex(void* p, size_t slotsize, size_t typesize, size_t count) ;

void*           hc_pool_resize_buffer_ex(void* p, size_t slotsize, size_t typesize, void* new_buffer, size_t new_buffer_size);
void*           hc_pool_grow_buffer_ex(void* p, size_t slotsize, size_t typesize, void* new_buffer, size_t new_buffer_size);
inline void     hc_pool_heap_free(void* p);

hc_pool_index   hc_pool_reserve_ex(void* pool, size_t slotsize, size_t typesize);
hc_pool_index   hc_pool_insert_ex(void* pool, size_t slotsize, size_t typesize, void* item, size_t size);
void            hc_pool_release_ex(void* pool, size_t slotsize, size_t typesize, hc_pool_index i);

#ifndef POOL_HEADER_ONLY

// creates Pool that fits into given buffer, lazy way to create a pool out of
// static buffer, for testing purpouses.

void hc_pool_from_buffer_ex(void* p, size_t slotsize, size_t typesize, void* buffer, size_t buffer_size) {
    assert(p && "expected valid pointer too pool");
    hc_PoolBase *pool = p;

    // TODO:
    (void) typesize;
    
    size_t indexsize        = sizeof(pool->free_list[0]);
    size_t full_typesize    = slotsize;
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


void hc_pool_from_heap_ex(void* p, size_t slotsize, size_t typesize, size_t count) {
    assert(p && "expected valid pointer too pool");
    assert(count && typesize);
    hc_PoolBase *pool = p;

    size_t item_size =      count * (sizeof(pool->items[0]));
    size_t free_list_size = count * (sizeof(pool->free_list[0]));
    assert(item_size && free_list_size);

    void* data = calloc(count, item_size + free_list_size); 
    hc_pool_from_buffer_ex(p, slotsize, typesize, data, item_size+free_list_size);
    pool->allocated_on_heap = true;
}



size_t hc_pool_measure_ex(size_t slotsize, size_t count) {
    const hc_PoolBase       pool;
    return count*slotsize 
         + count*sizeof(pool.free_list[0]);
}

void* hc_pool_resize_buffer_ex(void* p, size_t slotsize, size_t typesize, void* new_buffer, size_t new_buffer_size) {
    hc_PoolBase *pool   = p;
    hc_PoolBase copy    = *pool;
    void* old_ptr       = pool->items;

    void* old_items     = copy.items;
    void* old_free_list = copy.free_list;

    size_t old_items_size = 0,
           old_free_list_size = 0,
           new_items_size = 0,
           new_free_list_size = 0;

    old_items_size       = slotsize * pool->capacity;
    old_free_list_size   = sizeof(pool->free_list[0]) * pool->capacity;
    hc_pool_from_buffer_ex(pool, slotsize, typesize, new_buffer, new_buffer_size);
    new_items_size       = slotsize * pool->capacity;
    new_free_list_size   = sizeof(pool->free_list[0]) * pool->capacity;

    size_t items_size     = hc_pool_min(old_items_size, new_items_size);
    size_t free_list_size = hc_pool_min(old_free_list_size, new_free_list_size);

    if(old_items_size > new_items_size) {
        pool->max_count = pool->capacity;
        pool->count = pool->capacity;
    } else 
        pool->count = copy.count;

    memcpy(pool->items, old_items, items_size);
    memcpy(pool->free_list, old_free_list, free_list_size);

    return old_ptr;
}

void* hc_pool_grow_buffer_ex(void* p, size_t slotsize, size_t typesize, void* new_buffer, size_t new_buffer_size) {
    hc_PoolBase *pool = p;
    assert(hc_pool_measure_ex(slotsize, pool->capacity ) < new_buffer_size && "`pool_grow_buffer` was given smaller buffer size then before.");
    return hc_pool_resize_buffer_ex(p, slotsize, typesize, new_buffer, new_buffer_size);
}


inline void hc_pool_heap_free(void* p) {
    hc_PoolBase *pool = p;
    assert(pool->allocated_on_heap);
    if(pool->items) free(pool->items);
}

bool hc_pool_is_index_ok(hc_pool_index i) {
    return i != HC_POOL_INVALID_INDEX;
}

size_t hc_pool_assert_index(hc_pool_index i) {
    assert(hc_pool_is_index_ok(i) && "Attempt to access unset item");
    return i;
}

    
bool hc_pool_has_space(void* pool) {
    hc_PoolBase *p = pool;
    return p->count < p->capacity;
}

bool hc_pool_need_resize(void* pool) {
    hc_PoolBase *p = pool;
    return p->count >= p->capacity;
}

hc_pool_index hc_pool_reserve_ex(void* pool, size_t slotsize, size_t typesize) {
    hc_PoolBase *p = pool;
    assert(p->items && p->free_list && p->capacity && typesize);
    hc_pool_index index = (size_t)(-1);
    if (p->free_count > 0) {
        index = p->free_list[p->free_count-1];
        p->free_count--;
    } else {
        index = p->count;
    }
    assert(index != HC_POOL_INVALID_INDEX   && "Failed to reserve entity");
    assert(p->count < p->capacity           && "Attempt to buffer overflow");

    hc_PoolSlotBase* item = p->items + slotsize*index;
    item->infomask = HC_POOL_SLOT_IS_USED;
    p->count++;
    
    if (p->count > p->max_count) {
        p->max_count = p->count;
    }
    return index;
}

void hc_pool_release_ex(void* pool, size_t slotsize, size_t typesize, hc_pool_index i) {
    hc_PoolBase *p = pool;
    assert(p->items && p->free_list && p->capacity && typesize);
    if (p->count==0)
        return;
    assert(i < p->capacity && "Attempt to access Out of Bounds");

    hc_PoolSlotBase* item = (p->items + i*slotsize);

    if (!item->infomask) return;

    p->free_list[p->free_count] = i;
    p->free_count++;

    item->infomask = false;
    p->count--;
}


hc_pool_index hc_pool_insert_ex(void* pool, size_t slotsize, size_t typesize, void* item, size_t size) {
    hc_PoolBase *p = pool;
    assert(typesize == size);
    hc_pool_index i = hc_pool_reserve_ex(pool, slotsize, typesize);
    if(hc_pool_is_index_ok(i)) {
        void* slot      = p->items + slotsize*i;
        size_t offset   = (size_t)(slotsize-typesize);
        memcpy(slot + offset, item, size);
    }
    return i;
}
#endif/*  POOL_HEADER_ONLY   */


#ifdef   HC_POOL_STRIP_PREFIX
#define     pool_reserve        hc_pool_reserve
#define     pool_release        hc_pool_release
#define     pool_insert         hc_pool_insert
#define     pool_from_heap      hc_pool_from_heap
#define     pool_from_buffer    hc_pool_from_buffer
#define     pool_grow_buffer    hc_pool_grow_buffer
#define     pool_resize_buffer  hc_pool_resize_buffer

#define     pool_slot_size        hc_pool_slot_size   
#define     pool_type_size        hc_pool_type_size   
#define     pool_end              hc_pool_end
#define     pool_get              hc_pool_get
#define     pool_measure          hc_pool_measure
#define     pool_is_slot_used     hc_pool_is_slot_used

#define     pool_measure_ex           hc_pool_measure_ex
#define     pool_is_index_ok          hc_pool_is_index_ok
#define     pool_assert_index         hc_pool_assert_index
#define     pool_has_space            hc_pool_has_space
#define     pool_need_resize          hc_pool_need_resize

#define     pool_from_buffer_ex       hc_pool_from_buffer_ex
#define     pool_from_heap_ex         hc_pool_from_heap_ex

#define     pool_resize_buffer_ex     hc_pool_resize_buffer_ex
#define     pool_grow_buffer_ex       hc_pool_grow_buffer_ex
#define     pool_heap_free            hc_pool_heap_free

#define     pool_reserve_ex           hc_pool_reserve_ex
#define     pool_insert_ex            hc_pool_insert_ex
#define     pool_release_ex           hc_pool_release_ex

#define     pool_foreach              hc_pool_foreach

#endif /* HC_POOL_STRIP_PREFIX*/

#endif/* __HC_POOL_H    */
