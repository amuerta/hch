#include <stdio.h>
#include <hc_memory.h>
#include <hc_generic.h>
#include <hc_list.h>

void test_cons() {

    /*To make memory sanitizer stfu while keeping it for testing.*/
    Allocator alloc = allocator_temp();

    hc_Cons(int) *list, *iter, *tail, *next;

    int n = 1;
    for(n = n; n < 10; n++) {
        hc_cons_append_after(alloc, &list, n); 
    }

    tail = list;
    hc_cons_rewind_to_tail(&tail);

    printf("Tail (%p) = %i\n",tail, hc_cons_item(tail));

    printf("List: \n");
    iter = list;
    hc_cons_foreach(iter) {
        printf("%p = %i\n",iter, hc_cons_item(iter));
    }

    for(n = 0; n < 3; n++)
        hc_cons_remove_first(alloc, &list);

    hc_cons_rewind_forward(&list, 3);

    printf("List after i removed a bunch of stuff and rewinded by 3: \n");
    iter = list;
    hc_cons_foreach(iter) {
        printf("%p = %i\n", iter, hc_cons_item(iter));
    }
    
    next = list->next;
    hc_cons_drop(alloc, &list);
    printf("Dropped(list): %p\n",list);
    printf("Dropped(iter): %p\n",next->next);
}

void test_slist() {
    Allocator alloc = allocator_temp();

    hc_SList(int) list = {0};

    int n = 1;
    
    for(n = 1; n < 4; n++) 
        /*You can append expressions unless HC_LIST_NO_COMMA_OPERATOR_VALUES is defined.*/
        if(!hc_slist_append_head(alloc, &list, n*10)) {
            printf("Error occured when appending, most likely allocator error!\n");
            break;
        }

    hc_slist_rewind_forward(&list, 1);
    hc_slist_foreach(&list) {
        printf("%p = %i\n", list.iterator, hc_cons_item(list.iterator));
    }
    hc_slist_drop(alloc, &list);

}

int main(void) {

    printf("\t! hc_Cons example !: \n");
    test_cons();
    printf("\t! hc_SList example !: \n");
    test_slist();
    return 0;
}
