

// you define your data
typedef union {
    int as_int;
} SparseItem;


#define SET_TYPE_SAFETY
#define SET_EXTRA_TYPE_SAFETY

#include "set.h"

#if 0
int main(void) {
    Set s= {0};
    for(int i = 0; i < 10; i++) {
        set_append(&s, (SI){.as_int=i*10});
    }


    printf("SPARSE: \n [ ");
    for(int i = 0; i < 10; i++) {
        printf("%li ", s.sparse[i].de_id);
    }
    printf("]\n");

    printf("DENSE: \n [ ");
    for(int i = 0; i < 10; i++) {
        printf("%li ", s.dense[i].sp_id);
    }
    printf("]\n");

    printf("DATA: \n [ ");
    for(int i = 0; i < 10; i++) {
        printf("%i ", s.dense[i].data.as_int);
    }
    printf("]\n");

    set_free(&s);
}
#endif


#if 1

int main(void) {
    Set s = {0};

    int value = 0;

    set_assign_data_type(&s, int);

    for(int i = 0; i < 4; i++) {
        value = i * 10;
        set_append(&s, value , int);
    }
    set_remove(&s, 1);
    set_remove(&s, 3);
    set_remove(&s, 2);

    set_id myid = -1;

    value = 69;
    set_append(&s, value, int);

    value = 1339;
    myid = set_append(&s, value, int);

    value = 327;
    set_append(&s, value, int);


    printf("IDS:\n   ");
    for(size_t i = 0; i < s.capacity; i++) {
        printf("%2lu ", i);
    }
    printf("\n");

    printf("SPARSE: \n [ ");
    for(size_t i = 0; i < s.capacity; i++) {
        printf("%2li ", s.sparse[i].de_id);
    }
    printf("]\n");

    printf("DENSE: \n [ ");
    for(size_t i = 0; i < s.count; i++) {
        printf("%2li ", set_dense_get(&s, i)->sp_id);
    }
    printf("]\n");


    printf("Data: \n [ ");
    for(size_t i = 0; i < s.count; i++) {
        int* item = set_get(&s, i);
        printf("%2i ", item ? *item : -1);
    }
    printf("]\n");

    printf("1337 id: %li\n", myid);

    set_free(&s);
}
#endif
