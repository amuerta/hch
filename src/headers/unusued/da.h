/*
    DYNAMIC ARRAY
*/

#ifndef __HC_DA_H
#define __HC_DA_H

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
//  This way you can use hc_da_append outside of C, 
//  its pretty dirty with `void*` but you can create interfaces
//  in pretty much any other language other than C, or use macros of any kind.

#ifndef HC_DA_ITEMS_NAME
#   define HC_DA_ITEMS_NAME       items
#endif

#ifndef HC_DA_GROW_FACTOR
#   define HC_DA_GROW_FACTOR 2
#endif

#ifndef HC_DA_START_CAPACITY
#   define HC_DA_START_CAPACITY 32
#endif


// Simpler implementation means DA
// doesn't use header to store array metadata
// this ones relies on all items being in strict order
// as in DaGeneric.

// generic "interface"
typedef struct {
    void* items;
    size_t count, capacity, typesize;
} DaGeneric;

#define hc_da_append_macro(DA, VAR) do { \
    if ((DA)->capacity == 0) {\
        (DA)->capacity = HC_DA_START_CAPACITY;\
        (DA)->HC_DA_ITEMS_NAME = calloc(HC_DA_START_CAPACITY,sizeof(*(DA)->HC_DA_ITEMS_NAME));\
    }\
    if ((DA)->count >= (DA)->capacity) {\
        (DA)->capacity *= HC_DA_GROW_FACTOR;\
        (DA)->HC_DA_ITEMS_NAME = realloc((DA)->HC_DA_ITEMS_NAME,sizeof(*(DA)->HC_DA_ITEMS_NAME) * (DA)->capacity);\
    }\
    (DA)->HC_DA_ITEMS_NAME[((DA)->count)++] = VAR;\
} while(0);

void hc_da_append_generic(void* hc_da_ptr, void* varptr, size_t size) {
    DaGeneric* da = hc_da_ptr;
    if (da->capacity == 0) {
        da->capacity = HC_DA_START_CAPACITY; 
        da->items = calloc(HC_DA_START_CAPACITY , size);
        da->typesize = size;
    }
    assert(size == da->typesize);
    if (da->count >= da->capacity) {
        da->capacity *= HC_DA_GROW_FACTOR;
        da->items = realloc(da->items,size * da->capacity);
    }
    memcpy(da->items + (da->count * size), varptr, size);
    da->count++;
}


// Header Based implementation
// More robust and portable

typedef struct {
    size_t count, capacity, typesize;
} __DaHead__;
#define DaData __DaHead__ __hc_da_head__;
#define DaHead DaData

void hc_da_append_headed_generic(
        void** items_ptr, 
        __DaHead__ *head, 
        void* item, size_t tsize) 
{
    #define items (*items_ptr)
    if (head->capacity == 0 || !items) {
        head->capacity = HC_DA_START_CAPACITY; 
        items = calloc(HC_DA_START_CAPACITY,
                sizeof(tsize));
        head->typesize = tsize;
    }
    assert(tsize == head->typesize);
    if (head->count >= head->capacity) {
        head->capacity *= HC_DA_GROW_FACTOR;
        items = realloc(items,tsize * head->capacity);
    }
    memcpy(items + (head->count * tsize), item, tsize);
    head->count++;
    #undef items
}

#define hc_da_headed_append(A, it) hc_da_append_headed_generic(\
        ((void**) (&((A)->HC_DA_ITEMS_NAME))),\
        &((A)->__hc_da_head__),\
        &it, sizeof(it))
#define hc_da_headed_count(arr)       ((arr).__hc_da_head__.count)
#define hc_da_headed_capacity(arr)    ((arr).__hc_da_head__.capacity)
#define hc_da_headed_get(arr)         ((arr).HC_DA_ITEMS_NAME)



//
// INTERFACE
//


#ifdef HC_DA_MACRO_BASED

#define hc_da_append(A, it)        hc_da_append_macro(A, it)
#define hc_da_capacity(A)          ((A).capacity)
#define hc_da_get(A)               ((A).HC_DA_ITEMS_NAME)
#define hc_da_count(A)             ((A).count)
#define hc_da_foreach(A, iter)     for(size_t iter = 0; iter < hc_da_count(A); iter++)
#define hc_da_free(A)              free(A.HC_DA_ITEMS_NAME);

#else

#define hc_da_append(A, it)        hc_da_headed_append(A, it)
#define hc_da_capacity(A)          hc_da_headed_capacity(A)
#define hc_da_get(A)               hc_da_headed_get(A)
#define hc_da_count(A)             hc_da_headed_count(A)
#define hc_da_foreach(A, iter)     for(size_t iter = 0; iter < hc_da_count(A); iter++)
#define hc_da_free(A)              free(A.HC_DA_ITEMS_NAME);

#endif

// END
#endif// __HC_DA_H
/*
    END OF DYNAMIC ARRAY
*/
//
//
//
