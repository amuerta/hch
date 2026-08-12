
#define INCLUDE_ARENA
#define HCH_ARENA_IMPLEMENTATION
#include <hc_arena.h>

// cc -o NAME NAME.c -ggdb -pg -Wextra -Wall -fsanitize=address

void arena_test_flexible_block(void) {
    Arena a = {
        .default_block_size = 1024,
    };
    char *s = 0 ;
    void* ptr = 0;
    size_t size = 1<<12;


    s = arena_alloc(&a, size);
    for(size_t i =0; i < size-1; i++) 
        s[i] = '0'+(i % 10);
    // printf("<%s>\n", s);

    size *= 2;
    s = arena_alloc(&a, size);
    for(size_t i =0; i < size-1; i++) 
        s[i] = '0'+(i % 10);
    // printf("<%s>\n", s);



    size *= 4;
    s = arena_alloc(&a, size);
    for(size_t i =0; i < size-1; i++) 
        s[i] = '0'+(i % 10);
    // printf("<%s>\n", s);
    ptr = s;

    arena_realloc(&a, ptr, 1<<20);


    unsigned n = 0;
    printf("-----\n");
    for(ArenaBlock* it = a.memory; it; it = it->next, n++) 
        printf("#%u %p(%lu bytes)\n", n, it, it->block_size);

    arena_free(&a);
}


int arena_test_strings(void) {
    // Arena a = {0};
    
#if 1
    Arena a = {
        .default_block_size = 1<<12,
    };
#endif

    int count = 64;
    int size = 256;
    char** strings = 0;
    strings = arena_alloc(&a, sizeof(char*) * count);

    for(int i = 0; i < count; i++) {
        strings[i] = arena_alloc(&a, size);
        sprintf(strings[i], "string #%i", i);
    }

    for(int i = 0; i < count; i++) {
        printf("%s\n",strings[i]);
    }
    // rewind doesn't clear actual memory, just resets the node counter
    arena_rewind(&a);

    strings = arena_alloc(&a, sizeof(char*) * count);

    for(int i = 0; i < count; i++) {
        strings[i] = arena_alloc(&a, size);
        sprintf(strings[i], "string #%i", i+100);
    }

    for(int i = 0; i < count; i++) {
        printf("%s\n",strings[i]);
    }

#if 1
    // clear just both rewind and memset's block.data to 0

    printf("--- CLEARED ARENA ---\n");
    arena_clear(&a);
    for(int i = 0; i < count; i++) {
        printf("%s\n", strings[i]? strings[i] : "null");
    }

    // resets everything and frees the blocks
#endif
    arena_free(&a);


    return 0;
}

int main(void) {
    arena_test_flexible_block();
    arena_test_strings();
}
