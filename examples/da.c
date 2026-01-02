#define HC_DA_START_CAPACITY 2
#define HC_DA_ITEMS_NAME v
#define HC_DA_MACRO_BASED
#include "../src/da.h"

#define da_append hc_da_append
#define da_foreach hc_da_foreach
#define da_free hc_da_free
#define da_get hc_da_get

typedef struct {
    // you need items either way
    // you can redefine the name as anything before including da.h
    int* v;

    // this is needed for macro version.
    size_t count, capacity, typesize;

    // this is needed for header version
    DaHead;
} Numbers;

int main(void) {
    Numbers xs = {0};
    
    for(int i = 0; i < 128; i++) {
        da_append(&xs, i);
    }

    int one = -1, 
        two = -2 , 
        three = -3;
    da_append(&xs, one);
    da_append(&xs, two);
    da_append(&xs, three);

    da_foreach(xs, i) {
        printf("%i ", da_get(xs)[i]);
    }
    da_free(xs);
}
