#include <stdio.h>
#include <pool2.h>

typedef struct {
    int type;
    union {
        bool    Bool;
        int     Int;
        float   Float;
    } as;
} Object;

Object object(int t, int val) {
    Object o = {t, .as.Int = val};
    return o;
}

int main(void) {
    static char buffer [1<<10];

    hc_ResizablePool(Object) rpool = {0};
    hc_rpool_from_buffer(&rpool, buffer, sizeof(buffer));

    for(int i = 0; i < 10; i++)
        hc_rpool_insert(&rpool, object(1, i));

    hc_rpool_release_item(&rpool, 1);
    hc_rpool_release_item(&rpool, 2);
    hc_rpool_release_item(&rpool, 3);

    hc_rpool_reserve_item(&rpool);
    hc_rpool_reserve_item(&rpool);
    hc_rpool_reserve_item(&rpool);
    hc_rpool_reserve_item(&rpool);

    hc_rpool_release_item(&rpool, 3);
    hc_rpool_release_item(&rpool, 2);


    hc_RPoolIndex id = rpool.freelist;
    while(hc_rpool_strip_marker(id)) {
        printf("marker = %s, id = %u\n",
                hc_rpool_index_has_marker(id) ? "true" : "false",
                hc_rpool_strip_marker(id)
        );
        id = hc_rpool_derefrence_index((__hc_ResizablePool*)&rpool, sizeof(rpool.items[0]), id);
    }

    /*first item is not used.*/
    for(size_t i = 1; i < rpool.count; i++) {

        if(hc_rpool_index_has_marker(rpool.items[i].pointer))
            printf("pointer = %u\n", 
                    hc_rpool_strip_marker(rpool.items[i].pointer));
        else {
            Object o = rpool.items[i].data;
            printf("object = {t = %i, v = %i}\n", o.type, o.as.Int);
        }
    }
}
