#ifndef __HCH_BUMP_ALLOCATOR_H
#define __HCH_BUMP_ALLOCATOR_H
#include "stddef.h"
#include "assert.h"
#include "string.h"

typedef struct {
    size_t  flags;
    void    *start, *end;
    void    *pointer;
} Bump;

enum {
    BUMP_USE_PLATFORM_SIZED_ALIGNEMENT     = 1<<0,
} BumpAllocatorFlags;

#ifndef __HC_MEMORY_H
#   define hc_alignof(size, alignment)\
        (((size) + (alignment) - 1) & ~((alignment) - 1))
#endif /*__HC_MEMORY_H*/
Bump    hc_bump_from_buffer         (void*, size_t)         ;
void*   hc_bump_alloc               (Bump*, size_t)         ;
#define hc_bump_put(Bump, value)    hc_bump_memcpy(Bump, &(value), sizeof(value))
void*   hc_bump_memcpy              (Bump*, void*, size_t)  ;
void*   hc_bump_put_cstring         (Bump*, const char*)    ;
void    hc_bump_reset               (Bump*)                 ;

#ifdef __ALLOCATOR_INTERFACE
Allocator    allocator_bump(Bump*);
#endif

Bump hc_bump_from_buffer(void* buffer, size_t size) {
    Bump b = {
        .flags = BUMP_USE_PLATFORM_SIZED_ALIGNEMENT,
        .start = buffer,
        .end = buffer + size,
        .pointer = buffer,
    }; return b;
}


#ifdef __ALLOCATOR_INTERFACE
Allocator    allocator_bump(Bump* bump) {
    Allocator a = {
        .context_pointer = bump,
        .alloc = (void* (*) (void*, size_t)) hc_bump_alloc,
    }; return a;
}
#endif

void* hc_bump_alloc(Bump* bump, size_t size) {
    void* result = 0;
    assert(bump && "Passed invalid `Bump*`");
    
    size = hc_alignof(size, sizeof(size_t));
    if(!bump->pointer) {
        assert(bump->start && "Initilize the bump allocator start pointer.");
        bump->pointer = bump->start;
    }
    if(bump->pointer + size < bump->end)  {
        result         = bump->pointer;
        bump->pointer += size;
    }
    return result;
}

void* hc_bump_memcpy(Bump* bump, void* data, size_t size) {
    void* memory = hc_bump_alloc(bump, size);
    assert(bump && data && size && "Invalid argument(s) values.");
    memcpy(memory, data, size);
    return memory;
}

void* hc_bump_put_cstring(Bump* bump, const char* cstring) {
    return hc_bump_memcpy(bump, (void*) cstring, strlen(cstring));
}

void hc_bump_reset(Bump* bump) {
    assert(bump && "Passed invalid `Bump*`");
    bump->pointer = bump->start;
}

#endif/*__HCH_BUMP_ALLOCATOR_H*/
