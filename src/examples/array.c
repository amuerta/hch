#include <hc_memory.h>
#include <hc_generic.h>
#include <hc_array.h>

  
#define Array       hc_Array
#define da_append   hc_array_append_heap
#define Int(V)      (int)(V)

int main(void) {
    Allocator a_malloc = allocator_malloc();
    Array(int) xs = {0};
    int i = 0;

    for(i = 0; i < 10; i++) 
        hc_array_alloc_append(a_malloc, &xs, &i);
    for(i = 0; i < Int(xs.count); i++) 
        printf("%i ", xs.items[i]);

    hc_array_drop(a_malloc, &xs);
}
