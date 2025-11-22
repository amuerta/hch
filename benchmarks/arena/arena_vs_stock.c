#include "external/gc/src/log.h" 
#include "external/gc/src/log.c" 
#include "external/gc/src/gc.h" 
#include "external/gc/src/gc.c" 
#define ARENA_NODE_SIZE (4096*1024)
#include "../hc.h"

typedef struct Int {
    struct Int *next, *tail, *prev;
    int value;
} Int;

Int* arena_int(Arena* a, int n) {
    Int* item = arena_alloc(a, sizeof(Int));
    item->value = n;
    return item;
}

Int* malloc_int(int n) {
    Int* item = malloc(sizeof(Int));
    memset(item, 0, sizeof(*item));
    item->value = n;
    return item;
}

enum { ARG_ARENA, ARG_MALLOC, ARG_GC };

#define li_next(LI) ((LI)->next)
#define li_prev(LI) ((LI)->prev)

void benchmark(int length, int opt) {
    static Arena alloc;// = {0};
    Int* list = 0;
    Int* n = 0;


    for(int i = 0; i < length; i++) {
        if(opt == ARG_ARENA) {
            n = arena_int(&alloc, i);
            li_append(list, n);
        } else if (opt == ARG_MALLOC) {
            n = malloc_int(i);
            li_append(list, n);
        } else if (opt == ARG_GC) {
            n = gc_calloc(&gc, 1, sizeof(Int));
            n->value = i;
            li_append(list, n);
        }
    }
    

    int sum = 0, precsum = 0;
    for(int i = 0; i < length; i++) {
        precsum += i;
    }

    Int* iter = list;
    while(iter) {
        sum += iter->value;
        if(!li_next(iter)) break;
        iter = li_next(iter);
    }


    if (opt == ARG_MALLOC) {
        li_defer(list, Int, { // available: prev, next
            free(prev);
        });
    } else if (opt == ARG_ARENA) arena_free(&alloc);

    assert(sum == precsum);
}

int main(int argc, char** argv) {
    gc_start(&gc, &argv);
    const char* arg1 = argv[1];
    const char* arg2 = argv[2];
    int opt = ARG_ARENA;
    int length = 0;
    if (argc>=3) {
        length = atoi(arg2);
    }
    if(argc>=2) {
        if(!strcmp(arg1, "arena")) 
            opt = ARG_ARENA;
        else if (!strcmp(arg1, "malloc")) 
            opt = ARG_MALLOC;
        else if (!strcmp(arg1, "gc"))
            opt = ARG_GC;
    }
    if (opt != ARG_GC) gc_stop(&gc);
    benchmark(length,opt);
    if (opt == ARG_GC) gc_stop(&gc);
}
