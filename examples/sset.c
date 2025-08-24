

// you define your data
typedef union {
    int as_int;
} SparseItem;

#define SI SparseItem
#include "../src/sparse_set.h"


int main(void) {
    Set s = {0};
    for(int i = 0; i < 5; i++)
        set_append(&s, (SI){.as_int=i});

   set_remove(&s, 1);
   set_remove(&s, 3);
   set_remove(&s, 5);

   set_id myid = -1;

    set_append(&s, (SI){.as_int=69});
    myid = set_append(&s, (SI){.as_int=1337});
    set_append(&s, (SI){.as_int=228});

    printf("SPARSE: \n [ ");
    for(size_t i = 0; i < s.capacity; i++) {
        printf("%li ", s.sparse[i].de_id);
    }
    printf("]\n");

    printf("DENSE: \n [ ");
    for(size_t i = 0; i < s.dense_count; i++) {
        printf("%li ", s.dense[i].sp_id);
    }
    printf("]\n");


    printf("Data: \n [ ");
    for(size_t i = 0; i < s.dense_count; i++) {
        printf("%i ", s.dense[i].data.as_int);
    }
    printf("]\n");

    printf("1337 id: %li\n", myid);

    set_free(&s);
}
