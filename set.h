#ifndef __HCH_SET_H
#define __HCH_SET_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>

// TODO: come up with better solution then user defined struct / union
// for when you wan't to have high density item packing with wastly different sizes 
// of data stored in set

typedef long int set_id;

/* flexible array memeber */

#define SET_ITEM_HEAD_SIZE (sizeof(SetItem) - sizeof(unsigned char*))
typedef struct { set_id sp_id; unsigned char data[1]; } SetItem;

typedef struct  {
    const char* data_type;
    size_t      data_type_size;


    size_t count;   // count(dense) == count(sparse)
    size_t capacity;
    size_t free_count;

    set_id                                  *freelist;
    struct { set_id de_id; bool used; }     *sparse;
    SetItem                                 *dense; 
} Set;

void* set_recalloc(void* ptr, size_t prev, size_t neww) {
    void* new_ptr = calloc(neww, 1);
    if (!ptr) return new_ptr;
    
    memcpy(new_ptr, ptr, prev);
    free(ptr);
    return new_ptr;
}

#define SET_STARTING_CAPACITY 4

#define set_assign_data_type(S,T) set__assign_data_type((S),#T, sizeof(T))
void set__assign_data_type(Set* s, const char* t, size_t ts) {
    s->data_type = t;
    s->data_type_size = ts;
}

SetItem* set_dense_get(Set* s, size_t index) {
    //fprintf(stderr, "offset_size : %lu", (s->data_type_size + SET_ITEM_HEAD_SIZE));
    void* base = s->dense;
    return (SetItem*)(base + (s->data_type_size + SET_ITEM_HEAD_SIZE) * index);
}


void* set_get(Set* s, set_id id) {
    set_id deid = s->sparse[id].de_id;
    if (deid <= -1 || (size_t)deid >= s->count) return NULL; 
    return (void*)set_dense_get(s,s->sparse[id].de_id)->data;
}

void* set_get_or_panic(Set* s, set_id id) {
    assert(s->sparse[id].used);
    void* r = set_get(s, id);
    assert(r);
    return r;
}


#define set_append(S, I, T) set__append((S), &(I), sizeof((I)), #T)
set_id set__append(Set* s, void* item, size_t item_size, const char* type) {
    
    assert(s->data_type && s->data_type_size && "Assign data type first. EXAMPLE: set_assign_data_type(&set, int)");

    (void) (item);
    (void) (item_size);

    assert(s->data_type_size == item_size);

#ifdef SET_EXTRA_TYPE_SAFETY
    assert(strcmp(type, s->data_type)==0);
#endif//SET_SUPER_TYPE_SAFETY

    bool initilized = s->sparse && s->dense && s->freelist && s->capacity > 0;
    size_t free_size   = sizeof(*s->freelist);
    size_t sparse_size = sizeof(*s->sparse);
    size_t dense_size  = SET_ITEM_HEAD_SIZE + s->data_type_size;

    fprintf(stderr,"dense_size:  %lu\n", dense_size);
    fprintf(stderr,"sparse_size: %lu\n", sparse_size);
    fprintf(stderr,"free_size:   %lu\n", free_size);

    if (s->count + 1 > s->capacity) {
        if (!initilized) {
            s->capacity = SET_STARTING_CAPACITY;
            s->freelist = set_recalloc(0,0, s->capacity * free_size);
            s->sparse   = set_recalloc(0,0, s->capacity * sparse_size);
            s->dense    = set_recalloc(0,0, s->capacity * dense_size);
        } else {
            size_t new_capacity = s->capacity * 2;
            s->freelist = set_recalloc(s->freelist, s->capacity * free_size  , new_capacity * free_size  );
            s->sparse   = set_recalloc(s->sparse,   s->capacity * sparse_size, new_capacity * sparse_size);
            s->dense    = set_recalloc(s->dense,    s->capacity * dense_size , new_capacity * dense_size );
            s->capacity = new_capacity;
        }
    }

    set_id sparse = -1;
    set_id dense  = s->count;
    
    // check free list for slots
    if (s->free_count > 0) {
        sparse = s->freelist[s->free_count-1];
        s->free_count--;
    } else {
        sparse = s->count;
    }

    s->sparse[sparse].used  = true;
    s->sparse[sparse].de_id = dense;

    //s->dense [dense].data   = item;
    void* d = set_dense_get(s,dense)->data;
    memcpy(d, item, s->data_type_size);

    set_dense_get(s,dense)->sp_id = sparse;
    //s->dense [dense].sp_id  = sparse;

    s->count++;
    return sparse;
}

void set_memswap(void* l, void* r, size_t size) {
    assert(l && r);
    unsigned char temp[size];
    memcpy(temp, l,    size);
    memcpy(l,    r,    size);
    memcpy(r,    temp, size);
}



void set_remove(Set* s, set_id id) {
    assert((size_t)id < s->capacity);
    assert(s->sparse[id].used && "before calling set_remove, check if cell is valid with set_is_id_ok()");

    set_id dense  = s->sparse[id].de_id;
    set_id last   = s->count - 1;

    // item is last in dense buffer
    // just remove it and mark node as unused
    if ((size_t)dense == s->count - 1) {
        s->sparse[id].de_id   = -1;
        s->sparse[id].used    = false;
        //s->dense[dense].sp_id = -1;
        set_dense_get(s,dense)->sp_id = -1;
        s->count--;
        //printf("removing last\n");
    } else {
        // swap the elements data with their ids in sparce list
        // unlink left one, update the back link
        
        set_id sp_l = id;
        //set_id sp_r = s->dense[last].sp_id;
        set_id sp_r = set_dense_get(s,last)->sp_id;
        
        set_id de_l = dense;
        set_id de_r = s->count-1;

        void
            *ldat = set_dense_get(s,de_l)->data,
            *rdat = set_dense_get(s,de_r)->data
        ;

#if 0
        void
            *ldat = s->dense[de_l].data,
            *rdat = s->dense[de_r].data
        ;
#endif

        set_id
            *lid = &set_dense_get(s,de_l)->sp_id,
            *rid = &set_dense_get(s,de_r)->sp_id
        ;

        // left
        s->sparse[sp_l].de_id = -1;
        s->sparse[sp_l].used = false;
        set_dense_get(s,de_l)->sp_id = -1;

        // right
        s->sparse[sp_r].de_id = id;
    
        set_memswap(lid,  rid,  sizeof(*lid));
        set_memswap(ldat, rdat, s->data_type_size);

        s->freelist[s->free_count] = sp_l;
        s->free_count++;
        s->count--;
    }
}

void set_free(Set* s) {
    free(s->freelist);
    free(s->sparse);
    free(s->dense);
    memset(s, 0, sizeof(*s));
}

#endif//__HCH_SET_H
