#include <hc_memory.h>
#include <hc_generic.h>
#include <hc_array.h>

  
#define Array       hc_Array
#define da_append   hc_array_append_heap
#define Int(V)      (int)(V)

int main(void) {
    Allocator a_malloc = allocator_malloc();
    hc_Array(int) array = {0};
    int i = 0;

    for(i = 0; i < 100; i++) {
        /*If type of I doesn't match the size of Array type,
         * and error will happen.*/
        hc_array_alloc_append(a_malloc, &array, &i);
    }

    printf("array.count = %lu\n", array.count);

    printf("items: [ ");
    for(i = 0; (size_t)i < array.count; i++) {
        printf("%i ", array.items[i]);
    }
    printf("]\n");

    int count = array.count - 1;
    for(i = count; i >= 0; i--) {
        assert(hc_array_remove_unordered(&array, i));
    }

    printf("items after unordered_remove: [ ");
    for(i = 0; (size_t)i < array.count; i++) {
        printf("%i ", array.items[i]);
    }
    printf("]\n");

    hc_array_drop(a_malloc, &array);
    return 0;
}
