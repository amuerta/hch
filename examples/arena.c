
#define INCLUDE_ARENA
#define HCH_ARENA_IMPLEMENTATION
#include "../hc.h"

// cc -o NAME NAME.c -ggdb -pg -Wextra -Wall -fsanitize=address
int main(void) {
    Arena a = {0};
    
    // for testing
    #undef ARENA_NODE_SIZE
    #define ARENA_NODE_SIZE 2048

    int count = 64;
    int size = 256;
    char** strings = arena_alloc(&a, sizeof(char*) * count);

    for(int i = 0; i < count; i++) {
        strings[i] = arena_alloc(&a, size);
        sprintf(strings[i], "string #%i", i);
    }

    // rewind doesn't clear actual memory, just resets the node counter
    arena_rewind(&a);

    for(int i = 0; i < count/2; i++) {
        strings[i] = arena_alloc(&a, size);
        sprintf(strings[i], "string #%i", i+100);
    }

    for(int i = 0; i < count; i++) {
        printf("%s\n",strings[i]);
    }

    // clear just both rewind and memset's block.data to 0

    printf("--- CLEARED ARENA ---\n");
    arena_clear(&a);
    for(int i = 0; i < count; i++) {
        printf("%s\n", strings[i]? strings[i] : "null");
    }

    // resets everything and frees the blocks
    arena_free(&a);
}
