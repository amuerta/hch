#include "../src/memory.h"

#define format      hc_format
#define temp_alloc    hc_temp_alloc

int* get_array_of_xs(int n) {
    int *xs = 0;
    xs = temp_alloc(sizeof(int) * n);
    for(int i =0; i<n; i++)
        xs[i] = i;
    return xs;
}

int main(void) {
    for(int i = 0; i < 10000; i++) 
        printf("%s\n", format("string %5i", i));
    
    int n = 25;
    int* array = get_array_of_xs(n);
    printf("%s[%i] : [ ", "array" , n);
    for(int i = 0; i < n; i++) {
        printf("%i ", array[i]);
    }
    printf("]\n");
    return 0;
}
