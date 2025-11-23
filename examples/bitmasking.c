#include "../src/prelude.h"

enum {
    FLAG_1 = 1 << 0,
    FLAG_2 = 1 << 1,
    FLAG_3 = 1 << 2,
    FLAG_4 = 1 << 3,
};


int main(void) {
    int flags = 0b10010011;
    printf("bitmask: %b\n", flags);

    flags = bm_clear(flags, FLAG_2);
    printf("bitmask (cleared FLAG_2): %b\n", flags);

    flags = bm_set(flags, FLAG_2);
    printf("bitmask (set FLAG_2): %b\n", flags);

    flags = bm_toggle(flags, FLAG_2);
    printf("bitmask (toggle FLAG_2): %b\n", flags);
    flags = bm_toggle(flags, FLAG_2);
    printf("bitmask (toggle FLAG_2): %b\n", flags);

    printf("bitmask selection at four with size four: %b\n", (u32)bitmask_get_block(flags, 4, 4));
}
