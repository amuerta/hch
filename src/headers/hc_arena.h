/*      ![INFO] Simple implementation of Arena allocator. 
 *
 *  1. Allocator uses block of fixed or dynamic size, base size 
 * of the block is equal to size of memory page in linux (1024^2 * 4).
 *  2. Default block provider is malloc, but different providers can be chosen
 * (such as `VirutalAlloc` in windows or `mmap` in linux).
 *  3. Arena implementation supports resizing of allocations that occupy full
 * block worth of memory, as a way to use dynamic arrays with this allocator
 * and have all of its benefits. When used like so, allocator resembles a free-list.
 */

#ifndef __ARENA_H
#define __ARENA_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

// customize block size to your desire.
#define ARENA_DEFAULT_BLOCK_SIZE    (1024*1024*4)
#define ARENA_HEADER_SIZE           sizeof(ArenaBlock)

typedef unsigned char arena_bitmask8;
enum {
    ARENA_RESET_SIZE   = (1 << 0),
    ARENA_RESET_MEMORY = (1 << 1),
    ARENA_FREE_NODES   = (1 << 2),
};

typedef struct ArenaBlock {
    size_t              block_size, // total block size;
                        allocated; 
    struct ArenaBlock*  next;
    char                data[];
} ArenaBlock;

typedef struct {
    size_t      totally_allocated ;
    size_t      default_block_size;
    ArenaBlock*  memory;
} Arena;

// use these
void*       arena_alloc     (Arena*, size_t);
void*       arena_realloc   (Arena* a, void* ptr, size_t new_size);
void        arena_memcpy    (Arena* a, void* data, size_t size);
void        arena_reset     (Arena*, int opt);
#define     arena_put(A, I) arena_memcpy(A, &I, sizeof(I))
#define     arena_rewind(A) arena_reset(A, ARENA_RESET_SIZE)
#define     arena_clear(A)  arena_reset(A, ARENA_RESET_SIZE | ARENA_RESET_MEMORY)
#define     arena_free(A)   arena_reset(A, ARENA_RESET_SIZE | ARENA_RESET_MEMORY | ARENA_FREE_NODES)

// change this one for your specific need/enviorment/taste
ArenaBlock* arena_make_block(size_t);
void        arena_free_block(ArenaBlock*);

/*TODO: Create support for cross-platform mmap in memory.h and sbrk in linux*/
#ifndef ARENA_CUSTOM_NODE_ALLOCATOR
ArenaBlock* arena_make_block(size_t block_size) {
    assert(block_size > ARENA_HEADER_SIZE);
    ArenaBlock* block = malloc(block_size * 1);
    assert(block && "calloc failed");
    memset(block, 0, block_size);
    block->block_size = block_size;
    return block;
}

void arena_free_block(ArenaBlock* block) {
    assert(block); free(block);
}
#endif

/* 1) Allocate `size` in arena by placing it into memory block,
 * if block is too small to fit `size` create block that can hold exactly 
 * `size + ARENA_HEADER_SIZE` of bytes.
 * 2) If block available allocation space is getting too small to fit
 * `size` - create new node of ARENA_DEFAULT_BLOCK_SIZE or act as in (1). */
void* arena_alloc(Arena* arena, size_t size) {
    void* ret         = 0;
    ArenaBlock* tail   = arena->memory;
    size_t block_size = 0;

    if(!arena->default_block_size)
        arena->default_block_size = ARENA_DEFAULT_BLOCK_SIZE;
    block_size = arena->default_block_size;
    if(size > block_size) 
        block_size = size + ARENA_HEADER_SIZE;


    if (!arena->memory) {
        arena->memory = arena_make_block(block_size);
        tail = arena->memory;
        goto arena_alloc_goto;
    }

arena_alloc_goto:
    if (tail->allocated + size <= (tail->block_size - ARENA_HEADER_SIZE)) {
        ret = tail->data + tail->allocated;
        tail->allocated += size;
    } else {
        if(!tail->next)
            tail->next  = arena_make_block(block_size);
        tail            = tail->next;
        goto arena_alloc_goto;
    }
    return ret;
}

/*Realloc in arena is used only with FULL BLOCK allocations, 
 * if the allocation_size != block_size - HEAD_SIZE then
 * this fuction will assert.
 * This function creates a new block of bigger size
 * in place of the old one. This function exists for arrays 
 * that need to be able to resize and inherit all properties
 * of arenas, with respect to their lifetime.             */
void* arena_realloc(Arena* a, void* ptr, size_t new_size) {
    size_t      old_size, size_min;
    ArenaBlock   *prev, *replace_block = 0;
    for(ArenaBlock* it = a->memory; it; it = it->next) {
        if( it->data == ptr ) 
        {
            assert(it->allocated == it->block_size - ARENA_HEADER_SIZE
                    && "Only full-block sized allocations can be resized in Arena.");
            new_size = new_size + ARENA_HEADER_SIZE;
            old_size = it->block_size;
            size_min = (new_size > old_size) ? old_size : new_size;
            // create new block, copy data, swap pointers
            replace_block = arena_make_block(new_size);
            memcpy(it, replace_block, size_min);
            if(prev)
                prev->next = replace_block;
            arena_free_block(it);
            return replace_block;
        }
        prev = it;
    }
    assert(!"No blocks that match the pointer are found.");
}

void arena_reset(Arena* a, int opt) {
    a->totally_allocated = 0;
    ArenaBlock* block = a->memory;
    assert(block->block_size);

    while(block) {
        ArenaBlock* to_free = block;
        if (opt & ARENA_RESET_SIZE)
            block->allocated = 0;
        if (opt & ARENA_RESET_MEMORY)
            memset(block->data, 0, block->block_size - ARENA_HEADER_SIZE);

        block = block->next;
        if (opt & ARENA_FREE_NODES) 
            arena_free_block(to_free);
    }
}

void arena_memcpy(Arena* a, void* data, size_t size) {
    void* cell = arena_alloc(a, size);
    assert(cell && "Unexprected null, failed to allocate");
    memcpy(cell, data, size);
}


#endif//__ARENA_H
