/*
    DYNAMIC ARRAY
*/

#ifndef __DA_H
#define __DA_H

#include <stdlib.h>  
#include <stdio.h>   
#include <string.h>  
#include <assert.h>  

// it is just more useful then a standalone implementation 
// of dynamic array with void*
#ifndef da_append // if no da_append
                  // implement it

// This dynamic array is a trick i have seen @Tsoding use,
// and its (probably) the simplest way of doing dynamic array i used.
// 
// It's sort of robust and convinient to use.
// HOWEVER it's just a macro, which can be an issue when trying to port 
// projects/libraries to other languages.
//
// My solution:
//  Have api that either calls macros, or actual functions.
//  You can change which one to use with simple toggle.
//  Behaviour remains the same in both cases.
//
//  This way you can use da_append outside of C, 
//  its pretty dirty with `void*` but you can create interfaces
//  in pretty much any other language other than C, or use macros of any kind.
#ifndef DA_GROW_FACTOR
#   define DA_GROW_FACTOR 2
#endif


#ifndef DA_START_CAPACITY
#   define DA_START_CAPACITY 32
#endif

#ifdef DA_LINKABLE
#   define da_append(DA, VAR) \
        __da_append_generic(&((DA)->items), &(VAR), sizeof((VAR)));
#else
#   define da_append(DA, VAR) \
        __da_append_macro((DA), (VAR));
#endif

// generic "interface"
typedef struct {
    void* items;
    size_t count, capacity, typesize;
} DaGeneric;

#define __da_append_macro(DA, VAR) do { \
    if ((DA)->capacity == 0) {\
        (DA)->capacity = DA_START_CAPACITY;\
        (DA)->items = calloc(DA_START_CAPACITY,sizeof(*(DA)->items));\
    }\
    if ((DA)->count >= (DA)->capacity) {\
        (DA)->capacity *= DA_GROW_FACTOR;\
        (DA)->items = realloc((DA)->items,sizeof(*(DA)->items) * (DA)->capacity);\
    }\
    (DA)->items[((DA)->count)++] = VAR;\
} while(0);

void __da_append_generic(void* da_ptr, void* varptr, size_t size) {
    DaGeneric* da = da_ptr;
    if (da->capacity == 0) {
        da->capacity = DA_START_CAPACITY; 
        da->items = calloc(DA_START_CAPACITY ,sizeof(*da->items));
        da->typesize = size;
    }
    assert(size == da->typesize);
    if (da->count >= da->capacity) {
        da->capacity *= DA_GROW_FACTOR;
        da->items = realloc(da->items,size * da->capacity);
    }
    memcpy(da->items + (da->count * size), varptr, size);
    da->count++;
}



//
// QoL (im lazy)
//
#define DA_HEADER(T) \
        T* items;\
        size_t count, capacity, typesize;
#define DA_HEAD(T) DA_HEADER(T)
#define DA_IMPLEMENT(T) DA_HEADER(T)

#define da_loop(DA,I) for(size_t I = 0; I < DA.count; I++)

// in case i ever change API, if you use da_get, you should be fine.
#define da_get(DA) ((DA).items)

#endif// da_append 
#endif// __DA_H
