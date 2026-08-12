#ifndef __HCH_MAP_H
#define __HCH_MAP_H

#include <stdlib.h>  
#include <stdio.h>   
#include <string.h>  
#include <assert.h>  
#include <stdbool.h> 
#include <stdint.h>

// This map is intent-ed to be initialized by you
// for convenience there are `hc_map_heap` functions
// that allocate map via libc calloc, 
// i may add custom allocator interface in future, but for now you can just edit
// the source.
//
// Map is more lose then da.h or link.h is, for convenience, probably.
// By default it should be easy enough to use, but you have to 
// know what each macro/function does, so that you don't make mistakes.
// Memory sanitizer is advised if you wont use any memory trackers like free lists.
//
// If the map is allocated on the heap, it sets a marker that 
//  hc_map_free, hc_map_resize_generic,
//  hc_map_append_or_get
// use to check if everything is fine,
// IF NOT, they assert.

#ifndef HC_MAP_DEFAULT_INIT_SIZE
#   define HC_MAP_DEFAULT_INIT_SIZE 2048
#endif

//
// "API"
//
//  But tbh, this is just a macro wrapper around actual generic logic,
//  you can make kind of interface for map and make it as safe as you want, 
//  so i don't see a point making map any more difficult.
//
//  On the other hand it's good idea to have a limited sized map,
//  since its way easier to work with and rarely if ever you will need 
//  dynamically growling map. Cause why would you??


#define hc_Map(T) struct {\
    T* items;\
    MapHead map_head;\
}

#define hc_map_key_fmt(MS)  (int)(MS).count, (MS).items

#define hc_map_get_or_reserve(M, K) \
    ((M)->items[hc_map_get_or_reserve_generic(\
        &((M)->map_head), K\
    )])

#define hc_map_get(M, K) \
    ((M)->items[hc_map_get_generic(\
        &((M)->map_head), K\
    )])

/*
   TODO: consider making it a function so passing a pointer with size 
   is not that verbose when error occurs
*/

#define hc_map_dest_wrap(v)\
    v, (v) ? sizeof(*(v)) : 0

#define hc_map_ref_or_reserve_wrap(v, M, K) \
    (hc_map_get_or_reserve_generic(&(M)->map_head,\
            (void**) (&(M)->items),\
            (sizeof(*((M)->items))),\
            K,\
            hc_map_dest_wrap(v)\
    ))

#define hc_map_ref_wrap(v, M, K) \
    hc_map_get_generic(&(M)->map_head,\
            (void**) (&(M)->items),\
            (sizeof(*((M)->items))),\
            K,\
            hc_map_dest_wrap(v)\
        )

#define hc_map_grow(M, NEW_SIZE) \
    hc_map_grow_generic(&(M)->map_head,\
            (void**) (&(M)->items),\
            (sizeof(*((M)->items))),\
            NEW_SIZE)

#define hc_map_free(M) \
    hc_map_heap_free(&(M)->map_head);\
    free((M)->items);

//
// TYPES
//
typedef intmax_t sindex_t;

// Map uses slice instead of cstring
// for obvious reasons...
typedef struct {
    const char* items;
    size_t      count;
} MapKeySlice;

typedef struct {
    MapKeySlice*    keys;
    size_t          count, capacity, typesize;
    unsigned long (*hash1) (const char*, size_t);
    unsigned long (*hash2) (const char*, size_t);
    bool            heap_allocated; // if not assert on attempt to free
} MapHead;

// hash functions: 
// https://softwareengineering.stackexchange.com/questions/49550/which-hashing-algorithm-is-best-for-uniqueness-and-speed#145633
unsigned long djb2    (const char* str, size_t size, char shift);
unsigned long fnv1a   (const char* data, size_t size);

// Key
MapKeySlice hc_map_slice(const char* str, size_t count);
MapKeySlice hc_map_key(const char* str);

// map
static inline bool  hc_map_key_is_ok   (long int index);
float               hc_map_load        (MapHead  m); // in range from 0 to 1
long int            hc_map_query       (MapHead  m, MapKeySlice string);
long int            hc_map_take     (MapHead* m, MapKeySlice string);
void                hc_map_clear       (MapHead* m);
void                hc_map_calloc      (MapHead* m, size_t size);

long int            hc_map_get_or_reserve_generic(MapHead *head, MapKeySlice key);
long int            hc_map_get_generic(MapHead *head, MapKeySlice key);

unsigned long djb2(const char* str, size_t size, char shift) {
    unsigned long h = 5381;
    for (size_t i = 0; i < size; i++) 
        h = ((h << shift) + h) + str[i];
    return h;
}

unsigned long fnv1a(const char* data, size_t size) {
    unsigned long h = 2166136261UL;
    for (size_t i = 0; i < size; i++) {
        h ^= data[i];
        h *= 16777619;
    }
    return h;
}

unsigned long hash1   (const char* data, size_t size) {
    return fnv1a(data, size);
}
unsigned long hash2   (const char* data, size_t size) {
    return djb2(data, size, 33);
}


MapKeySlice hc_map_key(const char* str) {
    MapKeySlice s = {.items=str, .count=strlen(str)};
    return s;
}

MapKeySlice hc_map_slice(const char* str, size_t count) {
    MapKeySlice s = {.items=str, .count=count};
    return s;
}

static inline bool  hc_map_key_is_empty(MapKeySlice key) {
    return !key.items || key.count == 0;
}

static inline bool  hc_map_key_is_ok(long int index) {
    return index >= 0;
}


float hc_map_load(MapHead m) {
    assert(m.capacity);
    return (m.count == 0) ? 0 : (float)m.count/(float)m.capacity;
}

void hc_map_set_default_hashes(MapHead* m) {
    m->hash1 = hash1;
    m->hash2 = hash2;
}

void hc_map_calloc(MapHead* m, size_t cap) {
    if (cap == 0) m->capacity = HC_MAP_DEFAULT_INIT_SIZE;
    else m->capacity = cap;
    m->keys = calloc(m->capacity, sizeof(*m->keys));
    assert(m->keys && "CALLOC FAILED IN hc_map_calloc");
}

MapHead hc_map_heap(size_t cap, size_t typesize) {
    MapHead m = {0};
    hc_map_calloc(&m, cap);
    m.hash1 = hash1;
    m.hash2 = hash2;
    m.heap_allocated = true;
    m.typesize = typesize;
    return m;
}

void hc_map_clear(MapHead* m) {
    m->count = 0;
    m->capacity = 0;
    memset(m->keys, 0, sizeof(*(m->keys)) * m->capacity);
}

void hc_map_heap_free(MapHead* m) {
    assert(m->heap_allocated && 
            "MAKE SURE YOU MARK HEAP ALLOCATED MAP OR NOT ATTMEPT TO FREE NON HEAP MEMORY MAP.");
    if(m->keys) free(m->keys);
    memset(m, 0, sizeof(*m));
}


long int hc_map_take(MapHead* m, MapKeySlice string) {
    assert(m->keys && "EXPECTED TO HAVE KEYS INITILIZED");
    unsigned long 
        h1 = m->hash1(string.items, string.count),
        h2,
        index = -1
    ;
    MapKeySlice key = m->keys[(index = (h1 % m->capacity))];
    const size_t cap = m->capacity;

    // collision
    if (key.items) {
        h2 = m->hash2(string.items, string.count);
        // we hit again
        if (m->keys[(index = (h1+h2)% m->capacity)].items) {
            // "otherwise - iterative approach"
            for(size_t i = 0; i < m->capacity; i++) {
                if (!m->keys[(index = ((h1+h2)+i)%cap)].items) 
                    goto end;
            }
            return -1;
        } 
    } 

end:
    m->count++;
    if ((long int)index >= 0) m->keys[index] = string;
    return (long int) index;

}

long int hc_map_query(MapHead m, MapKeySlice string) {
#define mks_eq(k,s) (strncmp(k.items, s.items, s.count) == 0)
    // check if initlized
    assert(m.keys && "EXPECTED TO HAVE KEYS INITILIZED");
    if(!m.capacity) return -1;
    assert(string.items && string.count);

    unsigned long
        h1 = m.hash1(string.items, string.count),
        h2,
        index = -1
    ;
    size_t len = string.count;

    // check 1
    size_t cap = m.capacity;
    MapKeySlice key = m.keys[(index = h1 % m.capacity)];
    if (key.items && key.count == len && mks_eq(key, string)) 
        return (long int)index;

    // check 2 (double hash)
    h2 = m.hash2(string.items, string.count);
    key = m.keys[(index = (h1+h2)% cap)]; 

    if (key.items && key.count == len && mks_eq(key, string))  
        return (long int)index;

    //  try linear lookup  
    for(size_t i = 0; i < m.capacity; i++) {
        key = m.keys[(index = ((h1+h2)+i)%cap)]; 
        if(!key.items) break; // if we have a gap, this means 
                              // desired key can't be found here since
                              // if it would exist, it would be inserted in 
                              // linear fassion with current key,
                              // gap indicated end of this key lookup sequence
                              // or buggy/corrputed behaviour of the map
        if(key.count != len) continue;
        if (mks_eq(key, string))  
            return (long int)index;
    }

    return -1;
#undef mks_eq
}

long int hc_map_get_or_reserve_generic(MapHead* head, MapKeySlice key) 
{
    long int i = hc_map_query(*head, key);
    // doesn't exist
    if(i == -1) {
        i = hc_map_take(head, key);
        assert(i != -1 && "ATTEMPT TO APPEND TO FULL MAP, PERFORM CHECK FIRST");
    }
    return i;
}

long int hc_map_get_generic(MapHead* head, MapKeySlice key) 
{
    long int i = hc_map_query(*head, key);
    assert(i != -1 && "MAP DOESN'T HAVE THIS ELEMENT");
    return i;
}

void hc_map_grow_generic(MapHead* head, void** data, size_t typesize, size_t new_size) {
    assert(head->heap_allocated && "CANNOT GROW NON HEAP ALLOCATED MAP");
    assert(data && "EXPECTED DATA PTR TO BE VALID");
    assert(typesize == head->typesize && "MISMATCH IN MAP ITEMS TYPE SIZE AND GIVEN ITEM");
    assert(new_size > head->capacity && "MAP CAN ONLY GROW with hc_map_grow_generic");

    MapHead      new_head  = hc_map_heap(new_size, typesize);
    void*        new_items = calloc(new_size*typesize,          1);
    
    for(size_t i = 0; i < head->capacity; i++) {
        MapKeySlice key = head->keys[i];
        if(hc_map_key_is_empty(key)) continue;
        // if item exists, rehash it into new head 
        // and move item to new_items

        void* item = (*data) + typesize*i;
        void* new_item = (new_items) + 
            typesize *
            hc_map_get_or_reserve_generic(&new_head, key);

        memcpy(new_item, item, typesize);
    }
    
    // set new head
    hc_map_heap_free(head); // this will fail if not heap
    memcpy(head, &new_head, sizeof(*head));
    // set new data
    free(*data);            // so this is ok, probably.
    *data = new_items;
}

#endif//__HCH_MAP_H
