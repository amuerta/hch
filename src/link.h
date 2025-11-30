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

#ifdef LI_SIMPLER_IMPLEMENTATION
#define li_append(L, I) do {\
    if(!(L)) {(L) = (I); (L)->tail = (L);}\
    else {\
        void* __list_item__ = (I);\
        (L)->tail->next = __list_item__;\
        (L)->tail = __list_item__;\
    }\
}while(0)

#define li_next(LI) ((LI)->next)

#define li_foreach(LI, T, I, ...) do {\
   T* __next__ = (LI);\
   T* __prev__ = (LI); (void)__prev__; (void)__next__;\
   while(__next__) {\
       T* I = __next__;\
       {__VA_ARGS__}\
       __prev__ = __next__;\
       __next__ = __next__->next;\
   }\
} while(0)

#define li_defer(LI, T, ...) do {\
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
// More complex and sophisticated
//
#else 

typedef struct {
    size_t typesize;
    void *next, *prev, *tail;
} __ListData__;
#define ListHead ListData
#define ListData __ListData__ __head__;

#define li_append_macro(L, I) do {\
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

void li_append_generic(
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


#ifdef LI_LINKABLE
#   define li_append(list, item)\
        li_append_generic((void**)&(list),\
                item, sizeof(*item),\
                &((list)->__head__))
#else
#   define li_append(list, item)\
        li_append_macro(list, item)
#endif// LI_LINKABLE

#define li_next(list) (((list)->__head__).next)
#define li_prev(list) (((list)->__head__).prev)

#define li_foreach(list, type, iterator) \
    for(type iterator = list; iterator; iterator = li_next(iterator))


#endif//LI_SIMPLER_IMPLEMENTATION

// restore warning '-Wmissing-field-initializers'
#pragma GCC diagnostic pop

#endif//__LINK_H
