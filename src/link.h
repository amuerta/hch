#ifndef __LIST_H
#define __LIST_H

// remove useless warning 
// hide warning '-Wmissing-field-initializers'
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#include <stdlib.h>  
#include <stdio.h>   
#include <string.h>  
#include <assert.h>  

//
// Linked List (legacy)
//

#define hc_li_macro_append(L, I) do {\
    if(!(L)) {(L) = (I); (L)->tail = (L);}\
    else {\
        void* __list_item__ = (I);\
        (L)->tail->next = __list_item__;\
        (L)->tail = __list_item__;\
    }\
}while(0)

#define hc_li_macro_next(LI) ((LI)->next)
#define hc_li_macro_foreach(list, type, iterator) \
    for(type iterator = list; iterator; iterator = hc_li_next(iterator))


#define hc_li_macro_defer(LI, T, ...) do {\
   T* __next__ = (LI);\
   T* __prev__ = (LI);\
   while(__next__) {\
       __prev__ = __next__;\
       __next__ = __next__->next;\
       {__VA_ARGS__}\
       __prev__ = 0;\
   } (LI) = 0;\
} while(0)


// 
// LINK LIST HEADER BASED
//

typedef struct {
    size_t typesize;
    void *next, *prev, *tail;
} __ListData__;
#define ListHead ListData
#define ListData __ListData__ __head__;

#define hc_li_append_generic_macro(L, I) do {\
    if(!(L)) {(L) = (I); (L)->__head__.tail = (L);}\
    else {\
        size_t __head_offset__ = (void*)&((L)->__head__) - (void*)(L);\
        void* __list_item__ = (I);\
        void* __tail__ = (L)->__head__.tail;\
        /*Set prev item*/\
        (I)->__head__.prev = __tail__;\
        /*Set next item*/\
        ((__ListData__*)(__tail__ + __head_offset__))->next = __list_item__;\
        /*Set new tail*/\
        (L)->__head__.tail = __list_item__;\
    }\
}while(0)

void hc_li_append_generic_fn(
        void**listptr, 
        void* item, 
        size_t item_size, 
        void* list_header) 
{
    assert(listptr);
    assert(item);
    #define list (*listptr)
    __ListData__ *head = list_header;
    size_t offset = list_header - list;

    if (!list) { // pointer is null, the offset IS the list_header
        list = item;
        head = list + offset;
        head->tail = list;
        head->typesize = item_size;
    } 

    else {
        assert(head->typesize == item_size);
        head = list_header;
        __ListData__* tail_head = head->tail + offset;
        __ListData__* item_head = item + offset;
        // set previous
        void* tail = head->tail;
        item_head->prev = tail;
        // set next
        tail_head->next = item;
        head->tail = item;
    }

#undef list
}


#define hc_li_append_generic_wrap(list, item)\
        hc_li_append_generic_fn((void**)&(list),\
                item, sizeof(*item),\
                &((list)->__head__))

#define hc_li_generic_next(list) (((list)->__head__).next)
#define hc_li_generic_prev(list) (((list)->__head__).prev)

#define hc_li_generic_foreach(list, type, iterator) \
    for(type iterator = list; iterator; iterator = hc_li_next(iterator))

// TODOS

#define hc_li_defer_generic #error "TODO: implement defer generic"

//
// Interface
//

#ifdef HC_LI_MACRO_BASED

#define hc_li_append(list, item)\
        hc_li_macro_append(list, item)
#define hc_li_next(list) hc_li_macro_next(list)
#define hc_li_foreach(LI, T, I, ...) hc_li_macro_foreach(LI, T, I)
#define hc_li_defer(LI, T, ...) hc_li_macro_defer(LI, T, ...)

#else

#define hc_li_append(list, item)\
        hc_li_append_generic_wrap(list, item)
#define hc_li_next(list) hc_li_generic_next(list)
#define hc_li_foreach(LI, T, I) hc_li_generic_foreach(LI, T, I)
#define hc_li_defer(LI, T) hc_li_generic_defer()

#endif


// restore warning '-Wmissing-field-initializers'
#pragma GCC diagnostic pop
// END
#endif//__LINK_H
