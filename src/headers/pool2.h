#ifndef __HCH_POOL
#define __HCH_POOL

/* 
*/

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* [0b1000 ... ... 0000 ] vvvvvvvvvv */
#define HC_POOL_MARKER_FLAG         0x80000000
#define HC_POOL_REMOVE_MARKER_FLAG  0x7FFFFFFF
typedef unsigned hc_RPoolIndex;

/*

   TLDR:
    Pool that reuses slot memory to store nodes of freelist (singly linked list)
    Data structure trades a bit on general convenience and storage efficiency for 
    flawless resizing.


   The idea is that each slot is either data or pointer to next free slot 
   (0 if no other slot exists). Each free slot (even 0 one) is marked with
   flag 0x80000000 to indicate a free slot. If you try to index a slot
   that is free or to free it an assertion will fire.

   This isn't a prefect way to test for slot being free or not, best case would be to 
   go through each entry in free list and check if match with the item you are trying
   to get or double free, but I EXPECT YOU to structure your data in a way where miss-fires
   is impossible. 

   PROS:
    - everything within same buffer of memory.
    - resizing is easy, just copy old content to bigger buffer and you are done.
    - less code and macro hacking compare to splitting buffer into two arrays for free list and data.

   CONS:
    - requires acknowledgment of how it works, adapting to rules.
    - worse memory utilization as single big struct could fit many free indecies.
*/

#define hc_ResizablePool(T) struct {     \
    union {                     \
        T data;                 \
        hc_RPoolIndex pointer;   \
    } *items;                   \
    hc_RPoolIndex freelist;      \
    size_t count, capacity;     \
}

typedef struct {
    void* items;
    hc_RPoolIndex freelist;      
    size_t count, capacity;     
} __hc_ResizablePool;


#define hc_rpool_from_buffer(P, buffer, size) \
    __hc_rpool_from_buffer((void*)P, hc_rpool_typeinfo(P), buffer, size)

/* Passes both sizes as arguments */
#define hc_rpool_typeinfo(P)\
    sizeof((P)->items[0]), sizeof((P)->items[0].data)

/* Gets an id into a freshly made slot*/
#define hc_rpool_reserve_item(P)\
    __hc_rpool_alloc((void*)P, hc_rpool_typeinfo(P))

#define hc_rpool_insert(P, item) \
    ((P)->items[hc_rpool_reserve_item(P)].data) = item


hc_RPoolIndex hc_rpool_strip_marker(hc_RPoolIndex id) {
    return id & HC_POOL_REMOVE_MARKER_FLAG ;
}

hc_RPoolIndex hc_rpool_set_marker(hc_RPoolIndex id) {
    return id | (1<<31);
}

bool hc_rpool_index_has_marker(hc_RPoolIndex ptr) {
    return (bool) ((ptr & HC_POOL_MARKER_FLAG) >> 31);
}

hc_RPoolIndex hc_rpool_derefrence_index(__hc_ResizablePool* rpool, size_t slot_size, hc_RPoolIndex ptr) {
    hc_RPoolIndex markerless_pointer = hc_rpool_strip_marker(ptr);
    hc_RPoolIndex memory_under_pointer =
        *(hc_RPoolIndex*) (rpool->items + (slot_size * markerless_pointer));
    return memory_under_pointer;
}

void* hc_rpool_pointer_from_index(__hc_ResizablePool* rpool, size_t slot_size, hc_RPoolIndex index) {
    hc_RPoolIndex markerless_pointer = hc_rpool_strip_marker(index);
    void* pointer =
        (void*) (rpool->items + (slot_size * markerless_pointer));
    return pointer;
}


void __hc_rpool_from_buffer(void* rpool_ptr, size_t slotsize, size_t typesize, void* buffer, size_t size) {
    __hc_ResizablePool* rpool = (__hc_ResizablePool*) rpool_ptr;
    /*unused but passed to preserve argument convention */ 
    (void) typesize;

    rpool->capacity = (size/slotsize);
    rpool->count = 0;
    rpool->freelist = 0;
    rpool->items = buffer;
} 

hc_RPoolIndex __hc_rpool_alloc(void* rpool_ptr, size_t sizeof_slot, size_t sizeof_item) {
    __hc_ResizablePool* rpool = rpool_ptr;
    hc_RPoolIndex result = 0;
    unsigned head_content = 0;
    void* pointer_to_head_content = 0;

    /*unused but passed to preserve argument convention */ 
    (void) sizeof_item;

    assert(rpool && sizeof_slot && "This should'nt happen :/");
    assert(rpool->items && rpool->capacity && "Initilize the rpool!");
    
    /*check freelist pointer and update to next*/
    if(rpool->freelist) {
        result = hc_rpool_strip_marker(rpool->freelist);
        head_content = hc_rpool_derefrence_index(rpool, sizeof_slot, rpool->freelist);
        pointer_to_head_content = hc_rpool_pointer_from_index(rpool, sizeof_slot, rpool->freelist);

        assert(hc_rpool_index_has_marker(head_content) && "Freelist head should have 'freed' marker.");
        rpool->freelist = hc_rpool_strip_marker(head_content);

        // Make the slot zero_initlized
        memset(pointer_to_head_content, 0, sizeof_slot);
        return result;
    } 
    else {
        /*zero slot is considered to be an invalid index.*/
        if(!rpool->count) rpool->count++;
        return rpool->count++;
    }
}

#define hc_rpool_release_item(P, id) __hc_rpool_free((void*)P, hc_rpool_typeinfo(P), id)

hc_RPoolIndex __hc_rpool_free(__hc_ResizablePool* rpool,
        size_t sizeof_slot, size_t sizeof_type, 
        hc_RPoolIndex id) 
{
    hc_RPoolIndex freed = 0, next_in_list = 0, under_id = 0;
    void* cell_under_index = 0;
    (void) sizeof_type;

    if(id == rpool->count-1) {
        assert(rpool->count+1 && "Can't free when rpool is empty");
        rpool->count--;
    }
    else {
        /*Append free list with new item cell */
        id = hc_rpool_strip_marker(id);

        cell_under_index = hc_rpool_pointer_from_index(rpool, sizeof_slot, id);
        under_id = hc_rpool_derefrence_index(rpool, sizeof_slot, id);
        assert(!hc_rpool_index_has_marker(under_id) && "Double free/release_item detected.");

        memset(cell_under_index, 0, sizeof_slot);
        next_in_list = rpool->freelist;
        
        /*should only happen if next_in_list is zero*/
        if(!hc_rpool_index_has_marker(next_in_list))
            next_in_list = hc_rpool_set_marker(next_in_list);
        memcpy(cell_under_index, &next_in_list, sizeof(next_in_list));
        
        rpool->freelist = id;
    }
    return freed;
}

#endif/*__HCH_POOL*/
