#ifndef __HC_GENERIC_INTERFACE
#define __HC_GENERIC_INTERFACE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

/*Generic pointer with type information attached at runtime.*/
typedef struct {
    void const* pointer;
    struct {
        size_t      size;
        const char* name;
    } typeinfo;
} hc_TypedPointer;

#define hc_typedpointer(T, ptr) hc_typed_pointer(ptr,sizeof(*ptr),#T) 
hc_TypedPointer hc_typed_pointer(void* ptr, size_t typesize, const char* typename) {
    hc_TypedPointer p = {
        .pointer = ptr,
        .typeinfo = { .size =typesize, .name = typename}
    }; return p;
}

#define hc_typedpointer_unwrap(T, ptr)\
    (assert(\
            "Failed to unwrap hc_TypedPointer due to type missmatch." && \
            #T == (ptr).typeinfo.name && (ptr).typeinfo.size == sizeof(T)),\
            ((T*) ((ptr).pointer)))

typedef struct {       
    void   *collection, *collection_items;                     
    size_t collection_typesize;                        
    size_t collection_items_typesize;  
} hc_GenericCollection;

typedef struct {       
    void   *const collection, *const collection_items;                     
    size_t collection_typesize;                        
    size_t collection_items_typesize;                        
} hc_GenericCollectionReadOnly;

typedef struct {
    size_t  typesize;
    void*   item;
} hc_GenericValue;





hc_GenericValue hc_generic(void* item, size_t typesize) {
    hc_GenericValue v = {.item = item, .typesize = typesize};
    return v;
}

hc_GenericCollection hc_generic_collection(void* coll, void* coll_items, size_t typesize, size_t item_typesize) 
{
    hc_GenericCollection c = {
        .collection = coll, 
        .collection_items = coll_items,
        .collection_typesize = typesize,
        .collection_items_typesize = item_typesize
    }; return c;
}

void* hc_generic_unwrap(hc_GenericCollection collection, size_t expected_type_size)  {
    assert(collection.collection && 
            "Pointers to collection cannot be null");
    assert(collection.collection_typesize == expected_type_size &&
            "Type of collection and passed argument do not match type size.");
    return collection.collection;
}
void* hc_generic_validate_unwrap( hc_GenericCollection collection, hc_GenericValue value, size_t expected_type_size) 
{
    hc_generic_unwrap(collection, expected_type_size);
    assert(collection.collection_items_typesize == value.typesize &&
            "Items typesize and given value typesize do not match.");
    return collection.collection;
}

#endif/*__HC_GENERIC_INTERFACE*/
