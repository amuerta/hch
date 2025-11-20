//
// POOL datastructure
//

#include <stdlib.h>  
#include <stdio.h>   
#include <string.h>  
#include <assert.h>  

// TODO:? make it use not void* but user defined union/struct?

#ifndef __HCH_POOL_H
#define __HCH_POOL_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>


// TODO: use inline __asm__(int3) to have a proper breakpoint 
// instead of this old funny hack
// cause segmentaion fault to be able to run gdb on breakpoint
#ifdef DEBUG_SEGFAULT_ON_ASSERT
#   define FAULT_TRIGGER \
        *((int*)0) = 1 
#endif

#ifndef FAULT_TRIGGER
#define FAULT_TRIGGER // does nothing 
#endif

#ifndef hch_assert
#define hch_assert(COND,...) \
    do { if (!(COND)) { \
        fprintf(stderr,"Assertion at [%s:%s:%d]: ",__FILE__,__func__,__LINE__); \
        fprintf(stderr,__VA_ARGS__); \
        fprintf(stderr,"\n"); \
        FAULT_TRIGGER;      \
        fprintf(stderr, "NOTE: you can define FAULT_TRIGGER to enable gdb breakpoint\n");\
        exit(1);            \
    }} while(0)
#endif

typedef size_t              index_t;
typedef unsigned char       bitmask8;

#define INDEX_INVALID ((size_t)-1)

typedef enum {
	PoolState_allocated = 1,
} PoolState;

typedef struct {
    size_t      typesize;
    char*       type;

    void*       data;
    index_t*      free_indexes;
    

    void        (*destructor) (void*);
    size_t      capacity;
    size_t      count;
    size_t      free_count;
    size_t      max_size;
} Pool;

#ifndef POOL_MALLOC
#   define POOL_MALLOC(S) malloc(S)
#endif

#ifndef POOL_FREE
#   define POOL_FREE(P) free(P)
#endif

#ifndef POOL_REALLOC
#   define POOL_REALLOC(P,S) realloc(P,S)
#endif


#ifndef IGNORE_RETURN
#   define IGNORE_RETURN (void)
#endif

#define __POOL_typestring(T) #T

#ifndef POOL_DEFAULT_CAPACITY
#   define POOL_DEFAULT_CAPACITY 32
#endif

#define POOL_ITEM_POINTER(p,index) \
    p->data + index * (p->typesize + sizeof(bitmask8));



//      //
/* API  */
//      //

#define     pool_new(T)                         pool__init(NULL, POOL_DEFAULT_CAPACITY, sizeof(T), __POOL_typestring(T))
#define     pool_init(P,T,S)                    IGNORE_RETURN pool__init(P, S, sizeof(T), __POOL_typestring(T))
void        pool_resize(Pool* p, size_t newsize);
index_t     pool_reserve(Pool* p);
void        pool_release(Pool* p, index_t i);
void*       pool_refer(Pool* p, index_t i);


Pool pool__init(Pool* self, size_t capacity, size_t typesize, char* type) {
    const size_t data_sz_bytes = capacity * ( sizeof(bitmask8) + typesize );
    const size_t idxs_sz_bytes = capacity * sizeof(index_t);

    Pool new = {
        .type = type,
        .typesize = typesize,
        .capacity = capacity,
        .count = 0,
        .free_count = 0,
        .max_size = 0,

        // alloc memory,
        .data = POOL_MALLOC(data_sz_bytes),
        .free_indexes = POOL_MALLOC(idxs_sz_bytes),
    };

    if (!self) {
        return new;
    } 

    memcpy(self, &new, sizeof(new));
    return *self;
}

void pool_free(Pool* p) {
    free(p->data);
    free(p->free_indexes);
    memset(p,0,sizeof(*p));
}

void* pool_refer(Pool* p, index_t i) {
    hch_assert(p, "Expected to have valid pointer got NULL");
    hch_assert(p->data, "Expected to have valid data pointer initilized");
    void* ptr = POOL_ITEM_POINTER(p,i);
    bitmask8 state = *((bitmask8*)ptr);
    return (state) ? ptr : NULL;
}

void pool_resize(Pool* p, size_t newsize) {
    const size_t newsize_bytes = newsize * (sizeof(bitmask8) + p->typesize);
    const size_t newsize_indexes_bytes = newsize * sizeof(index_t);

    if (p->capacity == newsize) 
        return;
    else if (p->capacity > newsize) {
        // TODO:
        // impl destructor
    } 
 
    p->capacity     = newsize;
    p->data         = POOL_REALLOC(p->data,         newsize_bytes           );
    p->free_indexes = POOL_REALLOC(p->free_indexes, newsize_indexes_bytes   );
}

#define pool_append(P, VAR) \
    pool__append(P, &(VAR), sizeof(VAR))

index_t pool__append(Pool* p, void* data, size_t typesize) {
    index_t i = pool_reserve(p);
    if (i == INDEX_INVALID) 
        return INDEX_INVALID;

    hch_assert(typesize == p->typesize, 
            "Expected to have a type that equal or less than pool typesize");
    memcpy(pool_refer(p,i),data,typesize);
    return i;
}

index_t pool_reserve(Pool* p) {
    index_t index = INDEX_INVALID;
    if (p->free_count > 0) {
        index = p->free_indexes[p->free_count-1];
        p->free_count--;
    } else {
        index = p->count;
    }
    //debug("Reserved id: %u\n",ptr);
    hch_assert(index != INDEX_INVALID, "Failed to reserve entity");
    hch_assert(p->count < p->capacity, "Attempt to buffer overflow");

    bitmask8* state = POOL_ITEM_POINTER(p, index);
    *state |= PoolState_allocated;

    p->count++;
    if (p->count > p->max_size) {
        p->max_size = p->count;
    }
    return index;
}

void  pool_release(Pool* p, index_t i) {
    if (p->count==0)
        return;

    hch_assert(i < p->capacity, "Attempt to access Out of Bounds");
    bitmask8* state = POOL_ITEM_POINTER(p,i);

    if (!(*state & PoolState_allocated))
        return;
 
    p->free_indexes[p->free_count] = i;
    p->free_count++;

    *state ^= PoolState_allocated;
    p->count--;
}

#endif //__HCH_POOL_H
