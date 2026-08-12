/*
    DYNAMIC ARRAY
*/


/*
Example:

```c
   #include "array.h"
  
   #define Array       hc_Array
   #define da_append   hc_array_append_heap
   #define Int(V)      (int)(V)
  
   int main(void) {
       Allocator a_malloc = allocator_malloc();
       Array(int) xs = {0};
       int i = 0;
  
       for(i = 0; i < 10; i++) 
           hc_array_alloc_append(a_malloc, &xs, &i);
       for(i = 0; i < Int(xs.count); i++) 
           printf("%i ", xs.items[i]);
  
       hc_array_drop(&xs);
   }
```
*/

#ifndef __HC_ARRAY_H
#define __HC_ARRAY_H

#include <stdlib.h>  
#include <stdio.h>   
#include <string.h>  
#include <assert.h>  

#ifndef __ALLOCATOR_INTERFACE
#   error "type Array(T) requires allocator interface to work!"
#endif

#ifndef __HC_GENERIC_INTERFACE
#   error "generic type Array(T) requires generics interface to work!"
#endif

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#   define HC_INLINE_HINT static inline
#else
#   define HC_INLINE_HINT /*no inline in C89*/
#endif

typedef struct {
    void*  items;
    size_t count, capacity;
} hc_ArrayBase;

#ifndef HC_ARRAY_START_CAPACITY
#   define HC_ARRAY_START_CAPACITY 32
#endif

#ifndef HC_ARRAY_GROW_FACTOR
#   define HC_ARRAY_GROW_FACTOR 2
#endif

/*
 * API (MACROS)
 */

#define hc_Array(T) struct {\
    T* items;               \
    size_t count, capacity; \
}

/*Generic info constructing.*/
#define hc_array_collection_generic(A)\
    hc_generic_collection(          \
            (A),                    \
            (A)->items,             \
            sizeof(*(A)),           \
            sizeof((A)->items[0])   )

#define hc_array_value_generic(V)\
    hc_generic((V), sizeof((V)[0]))


/*Actual generics you would use.*/
#define hc_array_append(A, I)               \
    hc_array_append_generic(                \
            hc_array_collection_generic(A), \
            hc_array_value_generic(I))


#define hc_array_alloc_append(ALLOC, A, I)  \
    hc_array_alloc_append_generic(          \
            (ALLOC),                        \
            hc_array_collection_generic(A), \
            hc_array_value_generic(I))

#define hc_array_from_buffer(A, buffer, size)   \
    hc_array_from_buffer_generic(               \
            hc_array_collection_generic(A),     \
            buffer, size)

#define hc_array_resize(A,size)             \
    hc_array_resize_generic(                \
            hc_array_collection_generic(A), \
            size)

#define hc_array_drop(alloc, A)             \
    hc_array_drop_generic(                  \
            alloc,                          \
            hc_array_collection_generic(A)) \


#define hc_array_remove_unordered(A, idx)   \
    hc_array_remove_unordered_generic(      \
            hc_array_collection_generic(A), \
            idx)

/*
 * API (FUNCTIONS)
 */

typedef hc_GenericCollection hc_GenericArray;

void    hc_array_from_buffer_generic(hc_GenericArray, void* buffer, size_t size);
void    hc_array_resize_generic(Allocator, hc_GenericArray, size_t new_size);
bool    hc_array_needs_resize(hc_GenericArray arr);
void    hc_array_alloc(hc_GenericArray);
void    hc_array_drop_generic(Allocator alloc, hc_GenericArray arr_generic);


bool    hc_array_remove_unordered_generic(hc_GenericArray, size_t index);

bool    hc_array_append_generic(hc_GenericArray, hc_GenericValue);
bool    hc_array_alloc_append_generic(Allocator, hc_GenericArray, hc_GenericValue);

/*
 * IMPLEMENTATION
 */

#ifndef HC_ARRAY_HEADER_ONLY

void hc_array_from_buffer_generic(hc_GenericArray arr_generic, void* buffer, size_t size) {
    hc_ArrayBase* da = hc_generic_unwrap(arr_generic, sizeof(*da));
    size_t typesize = arr_generic.collection_items_typesize;
    size_t item_count = size/typesize;
    assert(da && "expected to have a valid pointer to Array(T)");
    assert(item_count > 0 && "buffer for array is too small to fit even 1 item");
    da->count = 0;
    da->capacity = item_count;
    da->items = buffer;
}

size_t hc_array_measure(size_t typesize, size_t count) {
    return typesize * count;
}

bool hc_array_needs_resize(hc_GenericArray arr_generic) {
    hc_ArrayBase* da = hc_generic_unwrap(arr_generic, sizeof(*da));
    return (da->count >= (da->capacity - 1)) || (da->capacity == 0);
}

/*TODO:
 *  This function is confusing, needs to be rewritten, eventually.
 */
void hc_array_resize_generic(Allocator allocator, hc_GenericArray arr_generic, size_t new_size_items) {
#ifndef HC_TYPES
    typedef char byte;
#endif

    hc_ArrayBase *array = hc_generic_unwrap(arr_generic, sizeof(*array));
    size_t typesize = arr_generic.collection_items_typesize;
    assert(array && "Expected to have a valid pointer to Array(T)");
    
    size_t 
        new_cap     = new_size_items,
        old_cap     = array->capacity,
        copy_count  = 0;
    byte *old_items = array->items,
         *new_items = NULL;
    
    size_t allocation_size = new_size_items * typesize;
    bool array_initilized = old_cap && old_items;
    if(!array_initilized || !new_size_items) {
        new_size_items = HC_ARRAY_START_CAPACITY;
        new_cap = new_size_items;
        allocation_size =  new_size_items * typesize;
    }

    /*If realloc interface is implemented and array alread initialized,
     *then we have an efficient way of resizing allocations.*/

    if(allocator.realloc && array_initilized) {
        size_t old_size = old_cap * typesize,
               new_size = new_cap * typesize;
        new_items = allocator_realloc(allocator, old_items, old_size, new_size);
    }
    /*If realloc not implemented, then it's a linear-esque
     * allocator and leaking stuff is considered fine.*/
    else {
        new_items = allocator_alloc(allocator, allocation_size);
    }

    assert(new_size_items > 0 && "new_size_cannot be 0.");
    assert(new_items && "allocator_alloc(Allocator, size_t) failed!");


    if(old_cap) {
        copy_count = (new_cap > old_cap) ? old_cap : new_cap;
        memcpy(new_items, old_items, copy_count * typesize);
    } else copy_count = new_cap;
    
    array->capacity = new_cap;
    array->items = new_items;

    /*Free if it's defined.*/
    if(old_items)
        allocator_free(allocator, old_items, old_cap);
}

void hc_array_drop_generic(Allocator allocator, hc_GenericArray arr_generic) {
    hc_ArrayBase *array = hc_generic_unwrap(arr_generic, sizeof(*array));
    if(array->items) allocator_free(allocator, array->items, array->capacity);
    memset(array, 0, sizeof(*array));
}

HC_INLINE_HINT void hc_array_byteswap(char* l, char* r) {
   *l = *r ^ *l;
   *r = *l ^ *r;
   *l = *r ^ *l;
}

bool hc_array_remove_unordered_generic(hc_GenericArray arr_generic, size_t index) {
    hc_ArrayBase *array = hc_generic_unwrap(arr_generic, sizeof(*array));
    size_t typesize = arr_generic.collection_items_typesize;
    if(index >= array->count || !array->count) return false;
    size_t i = 0, lastindex = array->count-1;

    if(index != lastindex) {
        char* item_to_remove = (char*)(array->items) + index*typesize;
        char* last_item      = (char*)(array->items) + lastindex*typesize;
        
        /* manual memswap */
        for(i = 0; i < typesize; i++) {
            char* l = (item_to_remove + i);
            char* r = (last_item      + i);
            hc_array_byteswap(l,r);
        }
    }


    array->count--;
    return true;
}


bool hc_array_append_generic(hc_GenericArray arr_generic, hc_GenericValue value) {
    typedef char byte;
    
    /* __asm__("int3"); */

    hc_ArrayBase    *array  = hc_generic_validate_unwrap(arr_generic, value, sizeof(*array));
    byte            *items  = array->items;
    size_t typesize         = arr_generic.collection_items_typesize;

    bool can_append = array->capacity && (array->count < array->capacity);
    /*
    assert(array->capacity && "Array(T) expected to be initilized");
    assert(array->count < array->capacity && "Faulty attempt to append into Array");
    */
    if(!can_append) goto end;

    memcpy(items + (array->count * typesize), value.item, value.typesize);
    array->count++;
end:
    return can_append;
}

bool hc_array_alloc_append_generic(Allocator allocator, hc_GenericArray arr_generic, hc_GenericValue value) {
    hc_ArrayBase* array = hc_generic_unwrap(arr_generic, sizeof(*array));
    
     /*__asm__("int3"); */
    if(hc_array_needs_resize(arr_generic)) 
        hc_array_resize_generic(allocator, arr_generic, 
                array->capacity * HC_ARRAY_GROW_FACTOR);
    return hc_array_append_generic(arr_generic, value);
}

#endif /* HC_ARRAY_HEADER_ONLY */
#endif /*__HC_ARRAY_H*/
