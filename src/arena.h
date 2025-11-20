/*
   ARENA
*/

// 
// Simple implementation of arena allocator build on top of the posix malloc
// you can replace malloc by a system specific memory allocator, like linuxe's mmap
// or window's get..memory..something..idk i dont use windows.
//

#ifndef __ARENA_H
#define __ARENA_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>


// customize block size to your desire.
#ifndef ARENA_NODE_SIZE // 2048, (4096*1024) - linux max page size
#   define ARENA_NODE_SIZE 4096 // 4kb is default linux page size. 
#endif

#define ARENA_HEADER_SIZE sizeof(ArenaNode)


typedef unsigned char arena_bitmask8;
enum {
    ARENA_RESET_SIZE   = (1 << 0),
    ARENA_RESET_MEMORY = (1 << 1),
    ARENA_FREE_NODES   = (1 << 2),
};

typedef struct ArenaNode {
    size_t              allocated;
    struct ArenaNode*   next;
    char                data[];
} ArenaNode;

typedef struct {
    size_t      totally_allocated;
    ArenaNode*  memory;
} Arena;


// use these
void*       arena_alloc     (Arena*, size_t);
void        arena_memcpy    (Arena* a, void* data, size_t size);
void        arena_reset     (Arena*, int opt);
#define     arena_put(A, I) arena_memcpy(A, &I, sizeof(I))
#define     arena_clear(A)  arena_reset(A, ARENA_RESET_SIZE | ARENA_RESET_MEMORY)
#define     arena_rewind(A) arena_reset(A, ARENA_RESET_SIZE)
#define     arena_free(A)   arena_reset(A, ARENA_RESET_SIZE | ARENA_RESET_MEMORY | ARENA_FREE_NODES)


// change this one for your specific need/enviorment/taste
ArenaNode* arena_make_node(void);



ArenaNode* arena_make_node(void) {
    return calloc(ARENA_NODE_SIZE, 1);
}

void* arena_alloc(Arena* a, size_t size) {
    assert(ARENA_NODE_SIZE > size);
    void* ret = 0;
    ArenaNode* tail = a->memory;
    
    if (!a->memory) {
        a->memory = arena_make_node();
        tail = a->memory;
        goto arena_alloc_goto;
    }
    while(tail->next) tail = tail->next;

arena_alloc_goto:
    if (tail->allocated + size < (ARENA_NODE_SIZE - ARENA_HEADER_SIZE)) {
        ret = tail->data + tail->allocated;
        tail->allocated += size;
    } else {
        tail->next = arena_make_node();
        tail = tail->next;
        goto arena_alloc_goto;
    }

    return ret;
}




void arena_reset(Arena* a, int opt) {
    a->totally_allocated = 0;
    ArenaNode* node = a->memory;
    
    while(node) {
        ArenaNode* to_free = node;
        if (opt & ARENA_RESET_SIZE)
            node->allocated = 0;
        if (opt & ARENA_RESET_MEMORY)
            memset(node->data, 0, ARENA_NODE_SIZE - ARENA_HEADER_SIZE);

        node = node->next;
        if (opt & ARENA_FREE_NODES) 
            free(to_free);
    }
}

void arena_memcpy(Arena* a, void* data, size_t size) {
    void* cell = arena_alloc(a, size);
    assert(cell && "Unexprected null, failed to allocate");
    memcpy(cell, data, size);
}


#endif//__ARENA_H
