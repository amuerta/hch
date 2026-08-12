#ifndef __HC_LEASH_H
#define __HC_LEASH_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#define hc_Leash(T) struct {        \
    size_t  count;                  \
    T*      items[sizeof(size_t)*8];\
    void*   (*alloc)(size_t);       \
    void    (*free)(void*);         \
}

#define lsh_typesize(leash) sizeof((leash)->items[0][0])

#define lsh_get(leash, I)\
    (leash.items[lsh_partition(I)][lsh_local_index(&leash, I)])

#define lsh_append(leash, I)\
    lsh_append_ex(leash, lsh_typesize(leash),\
            I, sizeof(*(I)) )

#define lsh_remove_unordered(leash, I)\
    lsh_remove_unordered_ex(leash, lsh_typesize(leash), (I))

typedef struct {
    size_t  count;
    void*   items[sizeof(size_t)*8];
    void*   (*alloc)(size_t);
    void    (*free)(void*);
} hc_LeashBase;

size_t lsh_local_index(void* leash, size_t index) {
    hc_LeashBase* l = leash;
    const size_t e = ceil(log2(index + 1));
    const size_t size  = 1<<e>>(!!index); // I love C
    const size_t local_index = index-size+!index;
    assert(index < l->count);
    return local_index;
}

static size_t lsh_partition(size_t index) {
    const size_t e = ceil(log2(index + 1));
    return e;
}

static inline void lsh_memswap(void* left, void* right, size_t size) {
    for(size_t i = 0; i < size; i++) {
        char* l = left  + i;
        char* r = right + i;
        *l = *r ^ *l;
        *r = *l ^ *r;
        *l = *r ^ *l;
    }
}


bool lsh_remove_unordered_ex(void* leash, size_t typesize, size_t index) {
    hc_LeashBase* l = leash;
    size_t last = l->count - 1;
    if(index >= l->count || !l->count) return false;
    // handle swaping;
    if(index != l->count) {
        void* item_this = l->items[lsh_partition(index)] + 
            lsh_local_index(l, index) * typesize;
        void* item_last = l->items[lsh_partition(last)] + 
            lsh_local_index(l, last) * typesize;
        lsh_memswap(item_this, item_last, typesize);
    }
    l->count--;
    return true;
}


void lsh_append_ex(void* leash, size_t typesize, void* item, size_t item_size) {
    hc_LeashBase* l = leash;
    const size_t index = l->count;
    const size_t e = ceil(log2(index + 1));
    const size_t size  = 1<<e>>(!!index); // I love C
    const size_t local_index = index-size+!index;
    const size_t offset = typesize*local_index;
    const size_t allocation_size = size*typesize;
    assert(typesize == item_size);
    if(!l->items[e]) l->items[e] = l->alloc ? 
        l->alloc(allocation_size) : calloc(allocation_size,1);
    memcpy((l->items[e]) + offset, item, item_size);
    l->count++;
}

void lsh_free(void* leash) {
    hc_LeashBase *l = leash;
    for(int i = 0; i < sizeof(l->items)/sizeof(l->items[0]); i++) 
        if(l->items[i]) l->free ? 
            l->free(l->items[i]) : free(l->items[i]);
}

int main(void) {
    hc_Leash(int) xs = {0};
    for(int i = 0; i < 10; i++) {
        lsh_append(&xs, &i);
        printf("%i\n", lsh_get(xs, i));
    }

    printf("removed unordered \n");
    int n = xs.count-1;
    for(int i = n; i >= 0; i--) {
        lsh_remove_unordered(&xs, i);
    }

    printf("Result:\n");
    for(int i = 0; i < xs.count; i++) 
        printf("%i\n", lsh_get(xs, i));
   
    lsh_free(&xs);
}

#endif /* __HC_LEASH_H */
