#ifndef __HCH_MAP_H
#define __HCH_MAP_H

#include <stdlib.h>  
#include <stdio.h>   
#include <string.h>  
#include <assert.h>  
#include <stdbool.h> 

#ifndef MAP_DEFAULT_INIT_SIZE
#   define MAP_DEFAULT_INIT_SIZE 2048
#endif

// Map uses slice instead of cstring
// for obvious reasons...
typedef struct {
    const char* items;
    size_t      count;
} MapKeySlice;

typedef struct {
    MapKeySlice*    keys;
    size_t count, capacity, typesize;
} Map;

#ifndef __HCH_PRELUDE_H
typedef intmax_t sindex_t;
#endif

// hash functions: 
// https://softwareengineering.stackexchange.com/questions/49550/which-hashing-algorithm-is-best-for-uniqueness-and-speed#145633
static inline unsigned long djb2    (const char* str, size_t size, char shift);
static inline unsigned long fnv1a   (const char* data, size_t size);

// Key
MapKeySlice map_slice(const char* str, size_t count);
MapKeySlice map_key(const char* str);

// map
static inline bool  map_key_is_ok   (long int index);
float               map_load        (Map  m); // in range from 0 to 1
long int            map_query       (Map  m, MapKeySlice string);
long int            map_reserve     (Map* m, MapKeySlice string);
void                map_clear       (Map* m);

#define maps_get    (M, S)                      maps_query   (M, map_key(S))
#define maps_put    (M, S)                      maps_reserve (M, map_key(S))
//      maps_resize (m, new_size, {CODE BLOCK}) /*macro*/

static inline unsigned long djb2(const char* str, size_t size, char shift) {
    unsigned long h = 5381;
    for (size_t i = 0; i < size; i++) 
        h = ((h << shift) + h) + str[i];
    return h;
}

static inline unsigned long fnv1a(const char* data, size_t size) {
    unsigned long h = 2166136261UL;
    for (size_t i = 0; i < size; i++) {
        h ^= data[i];
        h *= 16777619;
    }
    return h;
}

MapKeySlice map_key(const char* str) {
    MapKeySlice s = {.items=str, .count=strlen(str)};
    return s;
}

MapKeySlice map_slice(const char* str, size_t count) {
    MapKeySlice s = {.items=str, .count=count};
    return s;
}

static inline bool  map_key_is_ok   (long int index) {
    return index >= 0;
}

float map_load(Map m) {
    assert(m.capacity);
    return (m.count == 0) ? 0 : m.count/m.capacity;
}

Map map_alloc(Map* m, size_t cap) {
    static Map local_map;

    if(!m) m = &local_map;
    
    if (cap == 0) m->capacity = MAP_DEFAULT_INIT_SIZE;
    else m->capacity = cap;

    m->keys = calloc(m->capacity, sizeof(*m->keys));
    return *m;
}

// the idea is that for each map you implement, you call this 
// generic thing to implement map resize
//
// check example for details: ./examples/maps.c

// for now i don't handle undersizing the map.
#define map_resize(m, new_size, ...) do {\
    if (new_size <= (m)->capacity) break; \
    Map new_map = map_alloc(0, new_size);\
    for(size_t i = 0; i < (m)->capacity; i++) {\
        MapKeySlice key = (m)->keys[i];\
        if (!key.items || !key.count) continue;\
        long int oldid = (long int) i;\
        long int newid = map_reserve(&new_map, key);\
        assert(newid != -1 && "Should always be sucessful");\
        new_map.keys[newid] = key;\
        {__VA_ARGS__}\
    }\
    free((m)->keys);\
    m->keys = new_map.keys;\
    m->capacity = new_map.capacity;\
} while(0)

void map_clear(Map* m) {
    m->count = 0;
    m->capacity = 0;
    free(m->keys);
}


long int map_reserve(Map* m, MapKeySlice string) {
#define     hf1(c, size) (fnv1a(c, size))
#define     hf2(c, size) (djb2(c, size, 33))
    
    assert(m->keys && "call map_alloc() first");

    unsigned long 
        h1 = hf1(string.items, string.count),
        h2,
        index = -1
    ;
    MapKeySlice key = m->keys[(index = (h1 % m->capacity))];
    const size_t cap = m->capacity;

    // collision
    if (key.items) {
        h2 = hf2(string.items, string.count);
        // we hit again
        if (m->keys[(index = (h1+h2)% m->capacity)].items) {
            // "fuck it - iterative approach"
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

#undef  hf1
#undef  hf2
}

long int map_query(Map m, MapKeySlice string) {
 
#define hf1(c, size) (fnv1a(c, size))
#define hf2(c, size) (djb2(c, size, 33))
#define mks_eq(k,s) (strncmp(k.items, s.items, s.count) == 0)

    // initlized
    if(!m.capacity || !m.keys) return -1;
    //  good practice
    assert(string.items && string.count);


    unsigned long
        h1 = hf1(string.items, string.count),
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
    h2 = hf2(string.items, string.count);
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
#undef hf1
#undef hf2
}


#endif//__HCH_MAP_H


