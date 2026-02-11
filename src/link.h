#ifndef __LIST_H
#define __LIST_H

/*
 *  LINK DATA STRUCTURE:
 *  
 *      Simple linked list + N branch tree implementation
 *
 * Example:
    ```c
        #include "link.h"

        typedef struct Node {
            ImplementLink;
            int n;
        } Node;

        #define li_connect          hc_li_connect
        #define li_append           hc_li_append
        #define li_append_children  hc_li_append_children
        #define li_foreach          hc_li_foreach
        #define li_next             hc_li_next
        #define li_children         hc_li_children
        #define Link                hc_Link

        Node make_n(int n) {
            Node it = {0};
            it.n = n;
            return it;
        }

        void traverse_tree(Link(Node) it, int depth) {
            if(!it) return;
            for(int i = 0; i < depth; i++) printf("  ");
            printf("%i\n", it->n);

            Node* children = hc_li_children(it);
            while(children) {
                traverse_tree(children, depth + 1);
                children = li_next(children);
            }
        }

        int main(void) {
            Link(Node) tree = 0;
            Link(Node) handle = 0;

            Node
                root    = make_n(1),
                child1  = make_n(2),
                child2  = make_n(3),
                child3  = make_n(4),
                child4  = make_n(5),
                child5  = make_n(6)
            ;

            li_append(tree, &root);

            li_append_children(tree, &child1);
            li_append_children(tree, &child2);
            li_append_children(tree, &child3);
            li_append_children(tree, &child4);

            handle = li_children(tree);
            li_children(handle, &child5);
            
            traverse_tree(tree, 0);
        }
    ```
*/

#include <stdlib.h>  
#include <stdio.h>   
#include <string.h>  
#include <assert.h>  

// 
// LINK LIST HEADER BASED
//

// this strictly exists for self-documentation purposes.
#define hc_Link(T) T*

#define ImplementLink LinkData
#define LinkHead LinkData
#define LinkData __LinkData__ __head__;

//
// Interface
//

#define hc_li_insert(list, item)            hc_li_insert_wrap(list, item)
#define hc_li_append(list, item)            hc_li_append_wrap(list, item)
#define hc_li_append_children(list, item)   hc_li_append_children_wrap(list, item)
#define hc_li_connect(list, item)           hc_li_connect_wrap(list, item)

#define hc_li_next(list)        hc_li_generic_next(list)
#define hc_li_children(list)    hc_li_generic_children(list)
#define hc_li_parent(list)      hc_li_generic_parent(list)
#define hc_li_foreach(LI, T, I) hc_li_generic_foreach(LI, T, I)
#define hc_li_defer(LI, T)      hc_li_generic_defer()

enum {
    HC_LINK_NO_FLAGS     = 0,
    HC_LINK_INSERT_AFTER = (1<<0),
};

typedef struct {
    size_t typesize;
    void   *next,   *prev, 
           *head,   *tail,
           *children, *parent
    ;
} __LinkData__;

/* @doc 
 *  > Connects two link nodes together.
 * @doc*/
void* hc_li_connect_tail_fn(void* list,
        void* item, 
        size_t item_size, 
        void* list_header);

/* @doc 
 *  > Appends to link node as if its a list, expected pointer to node `*(*T)`
 *  > if `listptr` is NULL, sets appended node to be it.
 * @doc*/
void hc_li_append_fn(
        void**listptr, 
        void* item, 
        size_t item_size, 
        void* list_header,
        int opt);


/* @doc 
 *  > Appends to link->children node as if its a list, expected pointer to node `*(*T)`
 *  > if (*listptr) is NULL assert is triggered.
 * @doc*/
void hc_li_append_children_fn(
        void**listptr, 
        void* item, 
        size_t item_size, 
        void* list_header);


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

void hc_li_append_fn(
        void**listptr, 
        void* item, 
        size_t item_size, 
        void* list_header, 
        int opt) 
{
    typedef char byte;
    assert(listptr);
    assert(item);
    #define list (*listptr)

    void* after;
    __LinkData__ *head = list_header,
                 *item_head = 0, 
                 *tail_head = 0
    ;
    size_t offset = list_header - list;
    // pointer is null, 
    // the offset IS the list_header,
    // initilize new list metadata.
    if (!list) { 
        list = item;
        head = list + offset;
        head->tail = list;
        head->typesize = item_size;
    } 
    else {
        assert(head->typesize == item_size && "Missmatch in type sizes when appending");
        head      = (void*)((byte*)list         + offset);
        tail_head = (void*)((byte*)head->tail   + offset);
        item_head = (void*)((byte*)item         + offset);

        // if next exist, insert after 
        if(head->next && (opt & HC_LINK_INSERT_AFTER)) {
            after = head->next;
            /* item <- after*/
            ((__LinkData__*)(after + offset))->prev = item;
            /* list <- item -> after*/
            item_head->next = after;
            item_head->prev = list;
            /* list -> item */
            head->next = item;
        } 
        else {
            // set previous
            void* tail = head->tail;
            item_head->prev = tail;
            // set next
            tail_head->next = item;
            head->tail = item;
        }
    }

#undef list
}

void hc_li_append_children_fn(
        void**listptr, 
        void* item, 
        size_t item_size, 
        void* list_header) {
    assert(listptr);
    assert(item);
    #define list (*listptr)

    __LinkData__ *parent_head   = 0;
    __LinkData__ *item_head     = 0;
    size_t offset = list_header - list;
    int flags = 0;

    assert(list && "To append children you need to have node initilized.");
    parent_head = list + offset;
    
    assert(parent_head->typesize == item_size && "Missmatch in type sizes when appending");
    item_head   = item + offset;

    // set parent
    item_head->parent   = list;
    item_head->typesize = item_size;

    // __asm__("int3");
    flags = HC_LINK_NO_FLAGS;
    hc_li_append_fn(
            &(parent_head->children),  /*children ptr*/
            item, item_size, 
            (parent_head->children + offset),
            flags
    );

    #undef list
}


#define hc_li_insert_wrap(list, item)\
        hc_li_insert_fn((void**)&(list),\
                item, sizeof(*item),\
                &((list)->__head__), \
                HC_LINK_INSERT_AFTER)

#define hc_li_append_wrap(list, item)\
        hc_li_append_fn((void**)&(list),\
                item, sizeof(*item),\
                &((list)->__head__), \
                HC_LINK_NO_FLAGS)

#define hc_li_append_children_wrap(list, item)\
        hc_li_append_children_fn((void**)&(list),\
                item, sizeof(*item),\
                &((list)->__head__))


#define hc_li_generic_children(list)    (((list)->__head__).children)
#define hc_li_generic_parent(list)      (((list)->__head__).parent)
#define hc_li_generic_next(list)        (((list)->__head__).next)
#define hc_li_generic_prev(list)        (((list)->__head__).prev)

#define hc_li_generic_foreach(list, type, iterator) \
    for(type iterator = list; iterator; iterator = hc_li_next(iterator))

// TODOS

#define hc_li_defer_generic #error "TODO: implement defer generic"


#endif/*__LINK_H*/
