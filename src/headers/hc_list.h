#ifndef __HC_LIST_H
#define __HC_LIST_H

/* This list implementation has LEAST macro expansion layers
 * thus making it easy to read when fuck-up happens, while being 99% typesafe.
 * (And also convenient)
 *
 * You could argue that there is a better way, but honestly i don't care.
 * For a next better solution being 0.1% better while having 1000+ loc,
 * checking each compiler feature support or implementing a runtime 
 * reflections of types...
 * 
 * No.
 *
 * If you somehow shoot yourself in a foot with this List implementation. L.
 * Might as well just stop using old boomer language and program in something
 * that has proper generics.
 *
 * // TBD: Add my article url when I write one.
 */

/*TODO'S:
 *  - Add additional checking `for hc_TGeneirc`'s via asserts.
 *  - Implement `hc_Link` and `hc_DList` based on the link.
 *  - Consider adding `hc_Tree` and/or `hc_Branch`.
 *  - Utility? : list copying, searching via callback, slicing, etc.
 * */

#ifndef __HC_GENERIC_INTERFACE
#   error "List depends on Generic Interface implementation."
#endif

#ifndef __ALLOCATOR_INTERFACE
#   error "List depends on Allocator Interface implementation."
#endif


/*I learned about this trick from: 
 * (which in-turn was found by Sean Barrett and used for stb libraries.)
 *  https://danielchasehooper.com/posts/typechecked-generic-c-data-structures/
 * Genius, what can i say.
 * */
#ifndef hc_typecheck
#   define hc_typecheck(dest, src)\
        ((void)(1 ? (dest) : (src)), assert(sizeof(dest) == sizeof(src)))
#endif

typedef void hc_list_ignore;

#include <stdlib.h>
#include <assert.h>

/* TODO's:
 - finish constructor functionality.
 - implement Link, cons with 2 ends.
 - implement SList and DList.
 - implement Tree.. someday.
*/

/*
 * Constructor (Cons) is a simplest form of linked list - (Pointer, Data).
 * Despite it's simplicity - this form of list is probably the most common list
 * you'll need when prototyping or making fast, efficient storages using lists.
 *
 * For Double Linked Lists you need to look at `hc_Link`.
 *
 * CONS AND LINK ARE NOT LISTS IN THE SAME WAY THAT JAVA OR OTHER HIGH-LEVEL 
 * LANGUAGES PRESENT YOU THE LISTS! CONS AND LINK ARE SIMPLE CHAINS LIKE IN LISP WITHOUT
 * NOTION OF TAIL AND HEAD. OPERATIONS ON THEM ONLY VIEW CURRENTLY AVAILABLE NODE.
 * APPENDING TO TAIL OR ACCESSING ELEMENTS AT INDEX TAKES O(N) TIME BECAUSE EACH 
 * OPERATIONS REQUIRES SEARCHING FOR A DESIRED ELEMENT.
 *
 * IF YOU NEED LIST WITH ADDITIONAL PRE-HEADER THAT CONTAINS TAIL, HEAD, ETC - VIEW `hc_SList` 
 * FOR SINGLY LINKED LIST, and `hc_DList` FOR DOUBLY LINKED LIST.
 */

/******************************|hc_Cons|******************************/
typedef struct hc_ConsBase {
    struct hc_ConsBase  *next;
    char                data[1];
} hc_ConsBase;

typedef struct {
    hc_ConsBase **cons;
    size_t      cons_typesize;
} hc_ConsGeneric;

hc_ConsGeneric hc_cons_generic(hc_ConsBase**, size_t);
#define hc_cons_typeinfo(LIST) hc_cons_generic( \
            (hc_ConsBase**)(LIST),              \
            sizeof(**(LIST))                    \
        )   

/*One day I will make my own language to avoid being 
 * bothered by dump crap like this one. Boooo hoooo.*/
#ifdef HC_LIST_PEDANTIC_C89
#   define hc_Cons(T) union {                       \
        hc_ConsBase                     __internal; \
        struct { void* next; T item; }  get;        \
    }
#else
#   define hc_Cons(T) union {           \
        hc_ConsBase         __internal; \
        struct { void* next; T item; }; \
    }
#endif

/*Bo ho.*/
#ifdef HC_LIST_PEDANTIC_C89
#   define hc_list_item(L) ((L)->get.item)
#else
#   define hc_list_item(L) ((L)->item)
#endif

bool hc_cons_next_generic(hc_ConsGeneric);
bool hc_cons_rewind_to_tail_generic(hc_ConsGeneric);
bool hc_cons_append_after_generic(Allocator allocator, hc_ConsGeneric, hc_GenericValue value);
bool hc_cons_remove_first_generic(Allocator allocator, hc_ConsGeneric);

/* VOCAB:
 * L    - List (I call `Cons` and `Link` lists too), 
 * I    - item (Variable symbol, not rvalue (rhs)),
 * IT   - Iterator.
 * ALLOC- Allocator
 */

#define hc_cons_item \
    hc_list_item

#define hc_cons_foreach(IT)\
    for((IT) = (IT); (IT); hc_cons_next(&(IT)))

#define hc_cons_next(L)\
    hc__cons_next_generic(hc_cons_typeinfo(L))

#define hc_cons_rewind_to_tail(L)\
    hc__cons_rewind_to_tail_generic(hc_cons_typeinfo(L))

#define hc_cons_rewind_forward(L, IDX)\
    hc__cons_rewind_forward_generic(hc_cons_typeinfo(L),  IDX)

#define hc_cons_remove_first(ALLOC, L) \
    hc__cons_remove_first_generic(ALLOC, hc_cons_typeinfo(L))

/*Type check the item and Cons type, then assign value via copy.*/
/*Has warning message for when you try to pass a literall instead of 
 * symbol into `I`.*/
#define hc_cons_append_after(ALLOC, L, I) (         \
     hc_typecheck( hc_list_item(*(L)) ,  I),        \
     hc__cons_append_after_generic(ALLOC,           \
         /*(hc_ConsBase**)(L), sizeof(**(L)),*/     \
         hc_cons_typeinfo(L),                       \
         hc_generic(                                \
             &(I), /*This Constructor implementation doesn't support rvalues, sorry! ;<*/\
        sizeof(I))   )                              \
    )

/*Drops the entire list including starting Cons.*/
#define hc_cons_drop(ALLOC, L)\
    hc__cons_drop_generic(ALLOC, hc_cons_typeinfo(L))

/******************************|hc_Link|******************************/
typedef struct hc_LinkBase {
    struct hc_LinkBase  *next, *prev;
    char                data[1];
} hc_LinkBase;

typedef struct {
    hc_LinkBase **link;
    size_t      link_typesize;
} hc_LinkGeneric;

hc_LinkGeneric hc_link_generic(hc_LinkBase**, size_t);
#define hc_link_typeinfo(LIST) hc_link_generic(     \
            (hc_LinkBase**)(LIST),                  \
            sizeof(**(LIST))                        \
        )   

/*One day I will make my own language to avoid being 
 * bothered by dump crap like this one. Boooo hoooo.*/
#ifdef HC_LIST_PEDANTIC_C89
#   define hc_Link(T) union {                   \
        hc_LinkBase              __internal;    \
        struct { void* next; T item; }  get;    \
    }
#else
#   define hc_Link(T) union {           \
        hc_LinkBase         __internal; \
        struct { void* next; T item; }; \
    }
#endif

#ifndef HC_LIST_HEADER_ONLY

hc_ConsGeneric hc_cons_generic(hc_ConsBase **ptr, size_t typesize) {
    hc_ConsGeneric generic = {0};
    generic.cons = ptr; 
    generic.cons_typesize = typesize;
    return generic;
}

hc_ConsBase* hc_cons_alloc(Allocator allocator, size_t typesize) {
    return (hc_ConsBase*) memset(allocator_alloc(allocator, typesize), 0, typesize);
}

/*Appends to CURRENT node, shift next if present.
 * If you want to append to tail, first find it.*/
/*USE THIS AFTER YOU USED `hc_typecheck(a,b)`!!!*/
bool hc__cons_append_after_generic(Allocator allocator, hc_ConsGeneric cons_generic, hc_GenericValue value) {
    hc_ConsBase**   self                    = cons_generic.cons;
    size_t          cons_typesize           = cons_generic.cons_typesize;
    hc_ConsBase     *next_before_append     = NULL;
    
    assert(self && "Given NULL instead of pointer to Cons(T)");

    if(!(*self)) {
        (*self) = hc_cons_alloc(allocator, cons_typesize);
        /*append to head (first node)*/
        memcpy((*self)->data, value.item, value.typesize);
    } else {
        if((*self)->next) {
            next_before_append = (*self)->next;
            
            (*self)->next = hc_cons_alloc(allocator, cons_typesize);
            /*append to newly created node*/
            memcpy((*self)->next->data, value.item, value.typesize);
            
            (*self)->next->next = next_before_append;
        } 
        else {
            (*self)->next = hc_cons_alloc(allocator, cons_typesize);
            /*append to next*/
            memcpy((*self)->next->data, value.item, value.typesize);
        }
    }
    return true;
}

bool hc__cons_remove_first_generic(Allocator allocator, hc_ConsGeneric cons_generic) {
    hc_ConsBase     **self          = cons_generic.cons;
    size_t          cons_typesize   = cons_generic.cons_typesize;
    hc_ConsBase     *to_remove      = NULL;

    assert(self && "Given NULL instead of pointer to Cons(T)");
    if(!(*self)) return false;
    
    to_remove = (*self);
    if((*self)->next) {
        (*self) = (*self)->next;
    } else {
        (*self) = NULL;
    }

    if(allocator.free) 
        allocator_free(allocator, to_remove, cons_typesize);
    memset(to_remove, 0, sizeof(*to_remove));
    return true;
}

bool hc__cons_drop_generic(Allocator allocator, hc_ConsGeneric cons_generic) {
    hc_ConsBase     **self          = cons_generic.cons;
    size_t          cons_typesize   = cons_generic.cons_typesize;
    hc_ConsBase     *to_remove      = NULL;

    assert(self && "Given NULL instead of pointer to Cons(T)");
    if(!(*self)) return false;
    
    while((*self)) {
        to_remove = (*self);
        (*self) = (*self)->next;
        if(allocator.free) 
            allocator_free(allocator, to_remove, cons_typesize);
        memset(to_remove, 0, sizeof(*to_remove));
    }
    (*self) = NULL;
    return true;
}

bool hc__cons_rewind_forward_generic(hc_ConsGeneric cons_generic, size_t index) {
    hc_ConsBase     **self          = cons_generic.cons;

    assert(self && "Given NULL instead of pointer to Cons(T)");
    if(!(*self)) return false;

    size_t i = 0;
    while((*self)->next && (i < index)) {
        (*self) = (*self)->next;
        i++;
    }
    return true;
}

bool hc__cons_rewind_to_tail_generic(hc_ConsGeneric cons_generic) {
    hc_ConsBase     **self          = cons_generic.cons;

    assert(self && "Given NULL instead of pointer to Cons(T)");
    if(!(*self)) return false;

    while((*self)->next) (*self) = (*self)->next;
    return true;
}

bool hc__cons_next_generic(hc_ConsGeneric cons_generic) {
    hc_ConsBase     **self          = cons_generic.cons;

    assert(self && "Given NULL instead of pointer to Cons(T)");
    if(!(*self)) return false;
    (*self) = (*self)->next; return true;
}

/********************************|Single List|*********************************/
/*NOTE: SINGLE LIST DEPENDS ON IMPLEMENTATION OF `hc_Cons`, AS `hc_SList` IS 
 * JUST A CONVENIENT HEAD + TAIL WRAPPER AROUND `hc_Cons`. ATTEMPT TO USE IT
 * ALONG WILL RESULT IN MANY MANY MACRO EXPANSION AND UNDEFINED SYMBOL ERRORS!*/
/*Sink is pointer to node to `which hc_slist_append_*` writes*/
#define hc_SList(T) struct {\
    hc_Cons(T) *head, *tail, *iterator, *sink;\
}
#define hc_slist_item(L) (hc_cons_item( (L)->head ) )
#define hc_slist_sink(L) (hc_cons_item( (L)->sink ) )


typedef struct {
    hc_ConsBase **head;
    hc_ConsBase **tail;
    hc_ConsBase **iterator;
    hc_ConsBase **sink;
    size_t      typesize;
    bool        should_write_to_item;
} hc_SListGeneric;
const hc_GenericValue HC_SLIST_NO_VALUE_GENERIC = {0};

hc_SListGeneric hc_slist_generic(hc_ConsBase**, hc_ConsBase**, hc_ConsBase**, hc_ConsBase**, size_t);
#define hc_slist_typeinfo(LIST) hc_slist_generic(   \
            (hc_ConsBase**) &((LIST)->head),        \
            (hc_ConsBase**) &((LIST)->tail),        \
            (hc_ConsBase**) &((LIST)->iterator),    \
            (hc_ConsBase**) &((LIST)->sink),        \
            sizeof(*((LIST)->head))                 \
        )   

#define hc_slist_foreach(IT)\
    for((IT)->iterator = (IT)->head; (IT)->iterator; hc_cons_next( &((IT)->iterator) ))

#define hc_slist_remove_at_head(ALLOC, L) \
    hc__slist_remove_at_head_generic(ALLOC, hc_slist_typeinfo(L))

#define hc_slist_rewind_forward(L, IDX) \
    hc__slist_rewind_forward_generic(hc_slist_typeinfo(L), IDX)

#define hc_slist_drop(ALLOC, L) \
    hc__slist_drop_generic(ALLOC, hc_slist_typeinfo(L))


/*support appending expressions via comma operator.*/
#ifndef HC_LIST_NO_COMMA_OPERATOR_VALUES

#   define hc_slist_append_tail(ALLOC, L, I) (                              \
            hc_typecheck(hc_slist_item(L)  ,  I),                           \
            (hc__slist_append_tail_generic(ALLOC,                           \
                hc_slist_typeinfo(L), HC_SLIST_NO_VALUE_GENERIC),           \
            (((L)->sink)?                                                   \
             ((   hc_slist_sink(L) = I), true)  :false)             \
             )/*opreation was succesful if sink is not NULL (bug,edge case?)*/\
        )
/*  [item] ++ [3] -> [2] -> [1]  */
#   define hc_slist_append_head(ALLOC, L, I) (                              \
            hc_typecheck(hc_slist_item(L)  ,  I),                           \
            (hc__slist_append_head_generic(ALLOC,                           \
                hc_slist_typeinfo(L), HC_SLIST_NO_VALUE_GENERIC),           \
            (((L)->sink)?                                                   \
             ((  hc_slist_sink(L) = I), true)  :false)                      \
             )/*opreation was succesful if sink is not NULL (bug,edge case?)*/\
        )

#else
/*  [1] -> [2] -> [3] ++ [item]   */
#   define hc_slist_append_tail(ALLOC, L, I) (             \
         hc_typecheck(hc_slist_item(L)  ,  I),  \
         hc__slist_append_tail_generic(ALLOC,               \
             hc_slist_typeinfo(L),                          \
             hc_generic(                                    \
                 &(I), /*This List library implementation doesn't support rvalues, sorry! ;<*/\
            sizeof(I))   )                                  \
        )
/*  [item] ++ [3] -> [2] -> [1]  */
#   define hc_slist_append_head(ALLOC, L, I) (             \
         hc_typecheck(hc_slist_item(L) ,  I),  \
         hc__slist_append_head_generic(ALLOC,               \
             hc_slist_typeinfo(L),                          \
             hc_generic(                                    \
                 &(I), /*This List library implementation doesn't support rvalues, sorry! ;<*/\
            sizeof(I))   )                                  \
        )
#endif


void hc__slist_setup_first_node(Allocator allocator, hc_SListGeneric slist_generic, hc_GenericValue value);
bool hc__slist_append_tail_generic(Allocator allocator, hc_SListGeneric slist_generic, hc_GenericValue value);
bool hc__slist_append_head_generic(Allocator allocator, hc_SListGeneric slist_generic, hc_GenericValue value);
bool hc__slist_append_remove_at_head_generic(Allocator allocator, hc_SListGeneric slist_generic);

hc_SListGeneric hc_slist_generic(
        hc_ConsBase **head, hc_ConsBase **tail, 
        hc_ConsBase **iter, hc_ConsBase **sink, 
        size_t typesize) {
    hc_SListGeneric generic = {0};
    generic.head     = head; 
    generic.tail     = tail; 
    generic.iterator = iter; 
    generic.sink     = sink; 
    generic.typesize = typesize;
#ifndef HC_LIST_NO_COMMA_OPERATOR_VALUES
    generic.should_write_to_item = true;
#else
    generic.should_write_to_item = false;
#endif
    return generic;
}

void hc__slist_setup_first_node(Allocator allocator, hc_SListGeneric slist_generic, hc_GenericValue value) {
    hc_ConsBase**   head                    = slist_generic.head;
    hc_ConsBase**   tail                    = slist_generic.tail;
    hc_ConsBase**   sink                    = slist_generic.sink;
    size_t          typesize                = slist_generic.typesize;

    assert(tail && "Having tail but no head is weird, probably a bug! :/");
    (*head) = hc_cons_alloc(allocator, typesize);
    if(slist_generic.should_write_to_item)
        memcpy((*head)->data, value.item, value.typesize);
    (*sink) = (*head);
    (*tail) = (*head);
}

bool hc__slist_append_tail_generic(Allocator allocator, hc_SListGeneric slist_generic, hc_GenericValue value) {
    hc_ConsBase**   head                    = slist_generic.head;
    hc_ConsBase**   tail                    = slist_generic.tail;
    hc_ConsBase**   sink                    = slist_generic.sink;
    size_t          typesize                = slist_generic.typesize;
    hc_ConsBase     *temp                   = NULL;
    
    assert(head && "Given NULL instead of head pointer to Cons(T)*");

    /*If no head, make it, set tail to head.*/
    if(!(*head)) {
        hc__slist_setup_first_node(allocator, slist_generic, value);
        return true;
    }

    /*If head exists, so must the tail, 
     * append new node to tail.      */
    assert(tail && "Not having tail but having head is unintended. Probably a bug! :/");
    temp = hc_cons_alloc(allocator, typesize);
    if(slist_generic.should_write_to_item)
        memcpy(temp->data, value.item, value.typesize);
    (*sink) = temp;

    (*tail)->next = temp;
    (*tail) = temp;
    return true;
}

bool hc__slist_append_head_generic(Allocator allocator, hc_SListGeneric slist_generic, hc_GenericValue value) {
    hc_ConsBase**   head                    = slist_generic.head;
    hc_ConsBase**   tail                    = slist_generic.tail;
    hc_ConsBase**   sink                    = slist_generic.sink;
    size_t          typesize                = slist_generic.typesize;
    hc_ConsBase     *temp, *new_node        = NULL;
    
    assert(head && "Given NULL instead of head pointer to Cons(T)*");

    /*If no head, make it, set tail to head.*/
    if(!(*head)) {
        hc__slist_setup_first_node(allocator, slist_generic, value);
        return true;
    }

    /*If head exists, so must the tail, 
     * write current head to temp
     * make new_node head 
     * set head->next to temp
     */
    assert(tail && "Not having tail but having head is unintended. Probably a bug! :/");
    temp = (*head);

    new_node = hc_cons_alloc(allocator, typesize);
    if(slist_generic.should_write_to_item)
        memcpy(new_node->data, value.item, value.typesize);
    (*sink) = new_node;    

    (*head) = new_node;
    (*head)->next = temp;
    return true;
}

bool hc__slist_remove_at_head_generic(Allocator allocator, hc_SListGeneric slist_generic) {
    hc_ConsBase     **head          = slist_generic.head;
    size_t          typesize        = slist_generic.typesize;

    hc_ConsGeneric cons;
    cons.cons           = head;
    cons.cons_typesize  = typesize;
    return hc__cons_remove_first_generic(allocator, cons);
}

bool hc__slist_drop_generic(Allocator allocator, hc_SListGeneric slist_generic) {
    hc_ConsBase     **head          = slist_generic.head;
    size_t          typesize        = slist_generic.typesize;

    hc_ConsGeneric cons;
    cons.cons           = head;
    cons.cons_typesize  = typesize;
    return hc__cons_drop_generic(allocator, cons);
}

bool hc__slist_rewind_forward_generic(hc_SListGeneric slist_generic, size_t index) {
    hc_ConsBase     **head          = slist_generic.head;
    hc_ConsBase     **iterator       = slist_generic.head;
    size_t          typesize        = slist_generic.typesize;

    (*iterator) = (*head);

    hc_ConsGeneric cons;
    cons.cons           = iterator;
    cons.cons_typesize  = typesize;
    return hc__cons_rewind_forward_generic(cons, index);
}

#endif/*HC_LIST_HEADER_ONLY*/ 
#endif/*__HC_LIST_H*/
