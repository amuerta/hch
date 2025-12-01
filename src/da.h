/*
    DYNAMIC ARRAY
*/

#ifndef __DA_H
#define __DA_H

#include <stdlib.h>  
#include <stdio.h>   
#include <string.h>  
#include <assert.h>  

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


#ifndef DA_ITEMS_NAME
#   define DA_ITEMS_NAME       items
#endif

#ifndef DA_GROW_FACTOR
#   define DA_GROW_FACTOR 2
#endif

#ifndef DA_START_CAPACITY
#   define DA_START_CAPACITY 32
#endif


// Simpler implementation means DA
// doesn't use header to store array metadata
// this ones relies on all items being in strict order
// as in DaGeneric.
#ifdef DA_SIMPLER_IMPLEMENTATION

#ifdef DA_LINKABLE
#   define da_append(DA, VAR) \
        da_append_generic(&((DA)->DA_ITEMS_NAME), &(VAR), sizeof((VAR)));
#else
#   define da_append(DA, VAR) \
        da_append_macro((DA), (VAR));
#endif

// generic "interface"
typedef struct {
    void* items;
    size_t count, capacity, typesize;
} DaGeneric;

#define da_append_macro(DA, VAR) do { \
    if ((DA)->capacity == 0) {\
        (DA)->capacity = DA_START_CAPACITY;\
        (DA)->items = calloc(DA_START_CAPACITY,sizeof(*(DA)->items));\
    }\
    if ((DA)->count >= (DA)->capacity) {\
        (DA)->capacity *= DA_GROW_FACTOR;\
        (DA)->DA_ITEMS_NAME = realloc((DA)->DA_ITEMS_NAME,sizeof(*(DA)->DA_ITEMS_NAME) * (DA)->capacity);\
    }\
    (DA)->DA_ITEMS_NAME[((DA)->count)++] = VAR;\
} while(0);

void da_append_generic(void* da_ptr, void* varptr, size_t size) {
    DaGeneric* da = da_ptr;
    if (da->capacity == 0) {
        da->capacity = DA_START_CAPACITY; 
        da->DA_ITEMS_NAME = calloc(DA_START_CAPACITY , size);
        da->typesize = size;
    }
    assert(size == da->typesize);
    if (da->count >= da->capacity) {
        da->capacity *= DA_GROW_FACTOR;
        da->DA_ITEMS_NAME = realloc(da->DA_ITEMS_NAME,size * da->capacity);
    }
    memcpy(da->DA_ITEMS_NAME + (da->count * size), varptr, size);
    da->count++;
}

//
// QoL (im lazy)
//
#define DA_HEADER(T) \
        T* DA_ITEMS_NAME;\
        size_t count, capacity, typesize;
#define DA_HEAD(T) DA_HEADER(T)
#define DA_IMPLEMENT(T) DA_HEADER(T)

#define da_foreach(DA,I) for(size_t I = 0; I < DA.count; I++)
// in case i ever change API, if you use da_get, you should be fine.
#define da_get(DA)  ((DA).DA_ITEMS_NAME)
#define da_free(DA) free((DA).DA_ITEMS_NAME)
// END





#else
// Header Based implementation
// More robust and concise
#include <stdlib.h>
#include <assert.h>
#include <string.h>


typedef struct {
    size_t count, capacity, typesize;
} __DaHead__;
#define DaData __DaHead__ __da_head__;
#define DaHead DaData

void da_append_generic(
        void** items_ptr, 
        __DaHead__ *head, 
        void* item, size_t tsize) 
{
#define items (*items_ptr)
    if (head->capacity == 0 || !items) {
        head->capacity = DA_START_CAPACITY; 
        items = calloc(DA_START_CAPACITY,
                sizeof(tsize));
        head->typesize = tsize;
    }
    assert(tsize == head->typesize);
    if (head->count >= head->capacity) {
        head->capacity *= DA_GROW_FACTOR;
        items = realloc(items,tsize * head->capacity);
    }
    memcpy(items + (head->count * tsize), item, tsize);
    head->count++;
#undef items
}

typedef struct {
    int* items;
    DaHead;
} Ints;


#define da_append(A, it) da_append_generic(\
        ((void**) (&((A)->DA_ITEMS_NAME))),\
        &((A)->__da_head__),\
        &it, sizeof(it))
#define da_count(arr)       ((arr).__da_head__.count)
#define da_capacity(arr)    ((arr).__da_head__.capacity)
#define da_get(arr)         ((arr).DA_ITEMS_NAME)

#define da_foreach(A, iter) \
    for(size_t iter = 0; iter < da_count(A); iter++)
#define da_free(A) free(A.DA_ITEMS_NAME);


#endif // DA_SIMPLER_IMPLEMENATION

#endif// __DA_H
