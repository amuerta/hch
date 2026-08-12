#ifndef __HC_MEMORY_TRACKER_H
#define __HC_MEMORY_TRACKER_H

/*
 * Memory Tracker
 *
 *  Basic array of pointers to keep track of allocations done, 
 *  so that later when needed depending on order or allocation source
 *  you can free memory in bulk.
 *
 *  You can accomplish similiar results with arenas, but can be fine too.
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#ifndef HC_MEMORY_TRACKER_INITIIAL_CAPACITY
#   define HC_MEMORY_TRACKER_INITIIAL_CAPACITY 32
#endif

typedef struct {
    bool dynamically_allocated;
    struct { 
        int             kind;
        int             allocation_order;
        void*           allocation; 
    } *items;
    int highest_order;
    size_t count, capacity;
} hc_MemoryTracker;


#define hc_tracker_once(I)\
    for(void* tracker_counter = 0, I; (size_t)tracker_counter < 1; tracker_counter++)

// TODO: generative preprocesor templates for this one.
// Horendous bs that works fine for what i want to use it.
#define hc_tracker_free_match(MT, IT)                                       \
    for(int __trk_ord = (MT)->highest_order;__trk_ord > -1;__trk_ord--)     \
        for(size_t __trki = 0; __trki<(MT)->count;__trki++)                 \
            if((MT)->items[__trki].allocation_order == __trk_ord)           \
                hc_tracker_once(* IT = hc_mtracker_unplug((MT),__trki))     \
                    switch((MT)->items[__trki].kind)

void* hc_mtracker_unplug(hc_MemoryTracker* t, size_t index) {
    void* it;
    assert(index < t->count);
    it = t->items[index].allocation;
    t->items[index].allocation_order    = -1;
    t->items[index].allocation          = NULL;
    return it;
}

void    hc_mtracker_from_buffer (hc_MemoryTracker* t, void* buffer, size_t size);
void    hc_mtracker_from_heap   (hc_MemoryTracker* t);
void*   hc_mtracker_put         (hc_MemoryTracker* t, void* allocation, int kind, int order);
void*   hc_mtracker_put_dynamic (hc_MemoryTracker* t, void* allocation, int kind, int order);


void hc_mtracker_from_buffer(hc_MemoryTracker* t, void* buffer, size_t size) {
    t->items    = buffer;
    t->capacity = size / sizeof(t->items[0]);
    t->count    = 0;
}

void hc_mtracker_from_heap(hc_MemoryTracker* t) {
    t->items    = 0;
    t->capacity = 0;
    t->count    = 0;
    t->dynamically_allocated = true;
}

void* hc_mtracker_put(hc_MemoryTracker* t, void* allocation, int kind, int order) {
    assert(t->count < t->capacity);
    t->items[t->count].kind             = kind;
    t->items[t->count].allocation_order = order;
    t->items[t->count].allocation       = allocation;
    if(t->highest_order < order) t->highest_order = order;
    t->count++;
    return allocation;
}

void* hc_mtracker_put_dynamic(hc_MemoryTracker* t, void* allocation, int kind, int order) {
    if(t->count >= t->capacity) {
        size_t new_capacity = t->capacity ? t->capacity * 2 : HC_MEMORY_TRACKER_INITIIAL_CAPACITY;
        t->items = realloc(t->items, sizeof(t->items[0]) * new_capacity);
        t->capacity = new_capacity;
    }
    return hc_mtracker_put(t, allocation, kind, order);
}

#endif /*__HC_MEMORY_TRACKER_H*/
