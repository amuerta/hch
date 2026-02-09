#include "../src/array.h"

#define Array           hc_Array
#define da_append       hc_array_append_heap
#define da_remove_unord hc_array_remove_unordered
#define Int(V)          (int)(V)

int main(void) {
    Array(int) xs = {0};
    int i = 0;

    for(i = 0; i < 10; i++) 
        da_append(&xs, i);
 
    da_remove_unord(&xs, 2);
    da_remove_unord(&xs, 3);

    for(i = 0; i < Int(xs.count); i++) 
        printf("%i ", xs.items[i]);

    free(xs.items);
}
