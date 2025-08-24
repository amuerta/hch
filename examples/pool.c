#define INCLUDE_POOL
#include "../hc.h"

int main(void) {
    Pool p = pool_new(int);  

    pool_resize(&p,2);

    int data = 20;
    index_t i1 = pool_append(&p, data);


    data = 40;
    index_t i2 = pool_append(&p, data);

    int* d1 = pool_refer(&p,i1);
    int* d2 = pool_refer(&p,i2);
 
    printf("%lu : %i\n", i1, *d1);
    printf("%lu : %i\n", i2, *d2);
    
    pool_release(&p, i1);
    pool_release(&p, i2);
    pool_free(&p);
}
