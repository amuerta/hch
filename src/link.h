#ifndef __LIST_H
#define __LIST_H

#include <stdlib.h>  
#include <stdio.h>   
#include <string.h>  
#include <assert.h>  

// 
// LINK LIST HEADER BASED
//

// this strictly exists for self-documentation purposes.
#define hc_Link(T) T*

typedef struct {
    size_t typesize;
    void *next, *prev, *tail;
} __LinkData__;
#define ImplementLink LinkData
#define LinkHead LinkData
#define LinkData __LinkData__ __head__;

#define hc_li_append_generic_macro(L, I) do {\
    if(!(L)) {(L) = (I); (L)->__head__.tail = (L);}\
    else {\
        size_t __head_offset__ = (void*)&((L)->__head__) - (void*)(L);\
        void* __list_item__ = (I);\
        void* __tail__ = (L)->__head__.tail;\
        /*Set prev item*/\
        (I)->__head__.prev = __tail__;\
        /*Set next item*/\
        ((__LinkData__*)(__tail__ + __head_offset__))->next = __list_item__;\
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
    __LinkData__ *head = list_header;
    size_t offset = list_header - list;

    if (!list) { // pointer is null, the offset IS the list_header
        list = item;
        head = list + offset;
        head->tail = list;
        head->typesize = item_size;
    } 

    else {
        assert(head->typesize == item_size && "Missmatch in type sizes when appending");
        head = list_header;
        __LinkData__* tail_head = head->tail + offset;
        __LinkData__* item_head = item + offset;
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
/* END */

#endif/*__LINK_H*/
