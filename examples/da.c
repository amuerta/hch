
#include "../hc.h"

// EXAMPLE

// the header data: [items, count, capacity, typesize]

#if 0
typedef struct {
    int* items;
    size_t count, capacity, typesize;
    // ...
} Numbers;
#endif

// this is shortcut that does exactly the same
// as manual typedef with writing fields.
typedef struct {
    DA_HEADER(int);
} Numbers;

int main(void) {
    Numbers xs = {0};
    int one = 1, 
        two = 2 , 
        three = 3;
    da_append(&xs, one);
    da_append(&xs, two);
    da_append(&xs, three);

    da_loop(xs, i) {
        printf("%i ", da_get(xs)[i]);
    }

    free(xs.items);
}


