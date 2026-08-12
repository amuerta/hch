#include "bump.h"
#include "../src/memory.h"
#include <stdio.h>

int main(void) {
    static char memory[1<<14];
    Bump b = {.start = memory, .end = memory + sizeof(memory)};
    printf("%s\n", (const char*) bump_put_cstring(&b, "String!"));
    return 0;
}
