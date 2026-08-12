#ifndef __HC_EXPONENTIAL_ARRAY_H
#define __HC_EXPONENTIAL_ARRAY_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#ifndef __HC_MEMORY_H
#   define hch_alignof(size, alignment)\
        (((size) + (alignment) - 1) & ~((alignment) - 1))
    
    size_t hch_nearest_pow2(size_t x) {
        size_t p = 1;
        while(p < x) p*=2;
        return p;
    }

    bool hch_is_pow2(size_t n) {
        return (n & (n-1)) == 0;
    }
#endif

#ifndef __ALLOCATOR_INTERFACE
#   error "This data structure expects Allocator interface implementation."
#endif

typedef struct {
    size_t  count;
    size_t  block_size_multiplier;
    void*   items[sizeof(size_t)*8];
} __ExpoArray;

#define hc_ExpoArray(T) struct {    \
    size_t  count;                  \
    size_t  block_size_multiplier;  \
    T*      items[sizeof(size_t)*8];\
}

#define hc_xarray_typesize(exponential_array_ptr) sizeof((exponential_array_ptr)->items[0][0])

#define hc_xarray_get(exponential_array, I)\
    (exponential_array.items[hc_xarray_partition(&(exponential_array),I)]\
     [hc_xarray_local_index(&(exponential_array), I)])

#define hc_xarray_append(exponential_array_ptr, I)\
    hc_xarray_append_ex(exponential_array_ptr, hc_xarray_typesize(exponential_array_ptr),\
            I, sizeof(*(I)) )

#define hc_xarray_free(allocator, exponential_array_ptr)\
    hc_xarray_free_ex(allocator, exponential_array_ptr, hc_xarray_typesize(exponential_array_ptr))


#define hc_xarray_alloc(allocator, exponential_array_ptr, I)\
    hc_xarray_alloc_ex(allocator,exponential_array_ptr, hc_xarray_typesize(exponential_array_ptr), I)

#define hc_xarray_append_alloc(allocator, exponential_array_ptr, I)\
    hc_xarray_append_alloc_ex(allocator,exponential_array_ptr, hc_xarray_typesize(exponential_array_ptr),\
            I, sizeof(*(I)) )

#define hc_xarray_remove_unordered(exponential_array_ptr, I)\
    hc_xarray_remove_unordered_ex(exponential_array_ptr, hc_xarray_typesize(exponential_array_ptr), (I))


/*C89 doesn't have log2...*/
double hc_xarray_log2(double a) {
    return (log(a) / log(2));
}

size_t hc_xarray_local_index(void* exponential_array_ptr, size_t index) {
    __ExpoArray* l = exponential_array_ptr;
    /* const size_t e = ceil(hc_xarray_log2(index + 1)); */
    const double multiplier = l->block_size_multiplier ? l->block_size_multiplier : 1; 
    const size_t e = ceil(hc_xarray_log2(ceil( (double)(index + 1) / multiplier)));
    const size_t size  = (1<<e>>(!!index))          * multiplier; /* I love C */
    /*
       const size_t local_index = index-size+!index;
       */
    const size_t local_index = index%(size ? size :  (size_t)multiplier );
    assert(index < l->count);

    return local_index;
}

static size_t hc_xarray_partition(void* exponential_array_ptr, size_t index) {
    __ExpoArray* a = exponential_array_ptr;
    assert(hch_is_pow2(a->block_size_multiplier) && "Consider block sizes that are power of 2.");
    const double multiplier = a->block_size_multiplier ? a->block_size_multiplier : 1; 
    /* const size_t e = ceil(hc_xarray_log2(index + 1)); */
    const size_t e = ceil(hc_xarray_log2(ceil((double)(index + 1) / multiplier)));
    return e;
}

void hc_xarray_memswap(void* left, void* right, size_t size) {
    size_t i = 0;
    for(i = 0; i < size; i++) {
        char* l = left  + i;
        char* r = right + i;
        *l = *r ^ *l;
        *r = *l ^ *r;
        *l = *r ^ *l;
    }
}


bool hc_xarray_remove_unordered_ex(void* exponential_array_ptr, size_t typesize, size_t index) {
    __ExpoArray* l = exponential_array_ptr;
    size_t last = l->count - 1;

    if(index >= l->count || !l->count) return false;
    /* handle swaping; */
    if(index != l->count) {
        void* item_this = l->items[hc_xarray_partition(l, index)]; 
        void* item_last = l->items[hc_xarray_partition(l, last)];
        assert(item_this && item_last);
        item_this += hc_xarray_local_index(l, index) * typesize;
        item_last += hc_xarray_local_index(l, last) * typesize;
        hc_xarray_memswap(item_this, item_last, typesize);
    }
    l->count--;
    return true;
}

/*Alloc blocks up to a `index`*/
void hc_xarray_alloc_ex(Allocator allocator, void* exponential_array_ptr, size_t typesize, size_t index) {
    __ExpoArray* a = exponential_array_ptr;

    unsigned i = 0;
    double multiplier = a->block_size_multiplier ? a->block_size_multiplier : 1; 
    size_t e = ceil(hc_xarray_log2(ceil((double)(index + 1) / multiplier)));

    /* NOTE: Was a test code at some point.
    if(!l->items[e]) l->items[e] = l->alloc ? 
        l->alloc(allocation_size) : calloc(allocation_size,1);
    memcpy((l->items[e]) + offset, item, item_size);
    */

    for(i = 0; i <= e; i++) {
        if(!a->items[i]) {
            const size_t size  = (1<<i>>(!!i)) * multiplier; 
            const size_t allocation_size = size * hch_nearest_pow2(typesize) * multiplier;
            void* memory = allocator.alloc(allocator.context_pointer, allocation_size);
            a->items[i] = memory;
        }
    }

}

bool hc_xarray_append_ex(void* exponential_array_ptr, size_t typesize, void* item, size_t item_size) {
    __ExpoArray* l = exponential_array_ptr;
    const size_t index = l->count;

    /* I love C */
    const double multiplier = l->block_size_multiplier ? l->block_size_multiplier : 1; 
    const size_t e = ceil(hc_xarray_log2(ceil((double)(index + 1) / multiplier)));
    const size_t size  = (1<<e>>(!!index))                  * multiplier; 

    /*
       const size_t e = ceil(hc_xarray_log2(index + 1));
       const size_t size  = 1<<e>>(!!index); 
       const size_t local_index = index-size+!index;
       */
    const size_t local_index = index % (size ? size : (size_t)multiplier); /*1 % 1 = 0, so 1 % 2 = 1.*/
    const size_t offset = typesize*local_index;
    assert(typesize == item_size);
    
    /* NOTE: Was a test code at some point.
    const size_t allocation_size = size * hch_nearest_pow2(typesize) * multiplier;
    if(!l->items[e]) l->items[e] = l->alloc ? 
        l->alloc(allocation_size) : calloc(allocation_size,1);
    memcpy((l->items[e]) + offset, item, item_size);
    */
    /*If cannot append to block, do nothing, return false.*/
    if(!l->items[e]) {
        return false;
    }
    
    /*Otherwise append and return ok (true).*/
    memcpy((l->items[e]) + offset, item, item_size);
    l->count++;
    return true;
}


void hc_xarray_append_alloc_ex(Allocator allocator, 
        void* exponential_array_ptr, size_t typesize, 
        void* item, size_t item_size) 
{
    __ExpoArray* a = exponential_array_ptr;
    if(!hc_xarray_append_ex(a, typesize, item, item_size)) {
        hc_xarray_alloc_ex(allocator, a, typesize, a->count);
        assert(hc_xarray_append_ex(a, typesize, item, item_size)
                && "This shouldn't happen unless "
                "i messed something up" );
    }
}

void hc_xarray_free_ex(Allocator allocator, void* exponential_array_ptr, size_t typesize) {
    __ExpoArray *a = exponential_array_ptr;
    int i = 0;
    for(i = 0; i < sizeof(a->items)/sizeof(a->items[0]); i++) {
        /* Used to be a test code:
           if(l->items[i]) l->free ? 
           l->free(l->items[i]) : free(l->items[i]);
           */
        const bool first_item = (i == 0);
        const size_t e = i;
        const double multiplier = a->block_size_multiplier ? a->block_size_multiplier : 1; 
        const size_t size  = (1<<e>>(first_item)) * multiplier; 
        const size_t allocation_size = size * hch_nearest_pow2(typesize) * multiplier;
        void* ptr = a->items[i];
        if(ptr)
            allocator.free(allocator.context_pointer, a->items[i], allocation_size);
    }
}

#endif /* __HC_EXPONENTIAL_ARRAY_H */
