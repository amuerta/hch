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

/* TODO: implement custom alignment, for .. reasons? */
enum {
    BUMP_USE_PLATFORM_SIZED_ALIGNEMNT     = 1<<0,
} BumpAllocatorFlags;

#ifndef __HC_MEMORY_H
#   define hch_alignof(size, alignment)\
        (((size) + (alignment) - 1) & ~((alignment) - 1))
#endif

void*   bump_alloc              (Bump*, size_t)         ;
#define bump_put(bump, value)   bump_memcpy(bump, &(value), sizeof(value))
void*   bump_memcpy             (Bump*, void*, size_t)  ;
void*   bump_put_cstring        (Bump*, const char*)    ;
void    bump_reset              (Bump*)                 ;

void* bump_alloc(Bump* bump, size_t size) {
    void* result = 0;
    assert(bump && "Passed invalid `Bump*`");
    
    size = hch_alignof(size, sizeof(size_t));
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

void* bump_memcpy(Bump* bump, void* data, size_t size) {
    void* memory = bump_alloc(bump, size);
    assert(bump && data && size && "Invalid argument(s) values.");
    memcpy(memory, data, size);
    return memory;
}

void* bump_put_cstring(Bump* bump, const char* cstring) {
    return bump_memcpy(bump, (void*) cstring, strlen(cstring));
}

void bump_reset(Bump* bump) {
    assert(bump && "Passed invalid `Bump*`");
    bump->pointer = bump->start;
}

#endif/*__HCH_BUMP_ALLOCATOR_H*/
