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
       Array(int) xs = {0};
       int i = 0;
  
       for(i = 0; i < 10; i++) 
           da_append(&xs, i);
       for(i = 0; i < Int(xs.count); i++) 
           printf("%i ", xs.items[i]);
  
       free(xs.items);
   }
```
*/

#ifndef __HC_ARRAY_H
#define __HC_ARRAY_H

#include <stdlib.h>  
#include <stdio.h>   
#include <string.h>  
#include <assert.h>  

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
 * API (MACRO)
 */

#define hc_Array(T) struct {\
    T* items;               \
    size_t count, capacity; \
}

#define hc_array_typed(A) (A), sizeof(((A)->items)[0])
#define hc_array_append(A, I)           \
    hc_array_append_ex(          \
            hc_array_typed(A), \
            &I, sizeof(I))

#define hc_array_from_buffer(A, buffer, size)   \
    hc_array_from_buffer_ex(                    \
            hc_array_typed(A),                  \
            buffer, size)

#define hc_array_from_heap(A, count)            \
    hc_array_from_heap_ex(                      \
            hc_array_typed(A),                  \
            count)

#define hc_array_resize(A, newbuffer,size)      \
    hc_array_resize_ex(                         \
            hc_array_typed(A),                  \
            newbuffer, size)

#define hc_array_append_heap(A, I)              \
    hc_array_append_heap_ex(                    \
            hc_array_typed(A),                  \
            &I, sizeof(I))

#define hc_array_remove_unordered(A, I)\
    hc_array_remove_unordered_ex(\
            hc_array_typed(A), \
            I)

/*
 * API (FUNCTIONS)
 */

size_t  hc_array_measure        (size_t t, size_t count);

void    hc_array_from_buffer_ex (void* a, size_t t, void* buffer, size_t size);
void    hc_array_from_heap_ex   (void* a, size_t t, size_t count);

void hc_array_remove_unordered_ex(
    void* a, 
    size_t typesize,
    size_t index);

bool    hc_array_needs_resize   (void* a);
void*   hc_array_resize_ex(
    void* a,
    size_t typesize, 
    void* new_memory, 
    size_t size);

void    hc_array_append_ex(
    void* ptr, 
    size_t array_type_size, 
    void* varptr, 
    size_t size);

void hc_array_append_heap_ex(
    void* a, 
    size_t typesize, 
    void* item, 
    size_t size);

/*
 * IMPLEMENTATION
 */

#ifndef HC_ARRAY_HEADER_ONLY

void hc_array_from_buffer_ex(void* a, size_t typesize, void* buffer, size_t size) {
    hc_ArrayBase* da = a;
    size_t item_count = size/typesize;
    assert(a && "expected to have a valid pointer to Array(T)");
    assert(item_count > 0 && "buffer for array is too small to fit even 1 item");
    da->count = 0;
    da->capacity = item_count;
    da->items = buffer;
}

void hc_array_from_heap_ex(void* a, size_t typesize, size_t count) {
    size_t size = hc_array_measure(typesize,count);
    hc_array_from_buffer_ex(a,
            typesize, 
            calloc(size,1), 
            size);
} 

size_t hc_array_measure(size_t typesize, size_t count) {
    return typesize * count;
}

bool hc_array_needs_resize(void* a) {
    hc_ArrayBase* da = a;
    return (da->count >= (da->capacity - 1)) || (da->capacity == 0);
}

static inline void hc_array_byteswap(char* l, char* r) {
   *l = *r ^ *l;
   *r = *l ^ *r;
   *l = *r ^ *l;
}

void hc_array_remove_unordered_ex(
        void* a, 
        size_t typesize,
        size_t index) {
    
    hc_ArrayBase *array = a;
    size_t
        i = 0,
        lastindex = array->count-1;
    assert(index < array->count);
    if(index != lastindex) {
        char* item_to_remove = (char*)(array->items) + index*typesize;
        char* last_item      = (char*)(array->items) + lastindex*typesize;
        
        // manual memswap
        for(i = 0; i < typesize; i++) {
            char* l = (item_to_remove + i);
            char* r = (last_item      + i);
            hc_array_byteswap(l,r);
        }
    }


    array->count--;
}

void hc_array_append_heap_ex(void* a, size_t typesize, void* item, size_t size) {
    hc_ArrayBase *da = a;
    size_t cap      = da->capacity ? da->capacity*HC_ARRAY_GROW_FACTOR : HC_ARRAY_START_CAPACITY;
    size_t new_size = hc_array_measure(typesize, cap);
    
    if(hc_array_needs_resize(a)) 
        free(hc_array_resize_ex(a, typesize, calloc(new_size,1), new_size));
    hc_array_append_ex(a, typesize, item, size);
}

void hc_array_append_ex(void* ptr,
        size_t array_type_size, 
        void* varptr, 
        size_t size) 
{
    typedef char byte;
    
    hc_ArrayBase    *da     = ptr;
    byte            *items  = da->items;

    assert(da->capacity && "Array(T) expected to be initilized");
    assert(size == array_type_size);
    assert(da->count < da->capacity);
    
    memcpy(items + (da->count * size), varptr, size);
    da->count++;
}

void* hc_array_resize_ex(void* a, size_t typesize, void* new_memory, size_t size) {
    typedef char byte;

    hc_ArrayBase* da = a;
    size_t 
        new_cap     = size/typesize,
        old_cap     = 0,
        copy_count  = 0;
    byte *old_items = da->items,
         *new_items = 0;

    assert(a && "expected to have a valid pointer to Array(T)");
    assert(new_cap > 0 && "buffer for array is too small to fit even 1 item");

    old_cap   = da->capacity;
    new_items = new_memory;

    if(old_cap) {
        copy_count = (new_cap > old_cap) ? old_cap : new_cap;
        memcpy(new_items, old_items, copy_count * typesize);
    } else copy_count = new_cap;
    
    da->capacity = new_cap;
    da->items = new_items;

    return old_items;
}

#endif /* HC_ARRAY_HEADER_ONLY */
#endif /*__HC_ARRAY_H*/
