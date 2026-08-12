#include <hc_memory.h>

#define format        hc_format
#define temp_alloc    hc_temp_alloc

int* get_array_of_xs(int n) {
    int *xs = 0;
    xs = temp_alloc(sizeof(int) * n);
    for(int i =0; i<n; i++)
        xs[i] = i;
    return xs;
}

int main(void) {
    for(int i = 0; i < 1000; i++) 
        printf("%s\n", format("hc_format example #%i", i));
    
    /* size_t n = (1<<30); */
    size_t n = (1<<10); // page size that doesn't take eternity to print.
    SystemMemoryPage page = hc_memory_map(n, HC_MEMORY_DEFAULT); 
    page.size = n;
    assert(page.ptr);
    int* array = page.ptr;
    //get_array_of_xs(n);
#if 1
    printf("%s[%lu] : [ ", "array" , n);
    for(size_t i = 0; i < n; i++) {
        printf("%i ", array[i]);
    }
    printf("]\n");
#endif
    assert(hc_memory_unmap(&page));
    return 0;
}
