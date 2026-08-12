#ifndef __HCH_TABLE
#define __HCH_TABLE
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

/* C Hash Table. (Written in and for C89 and beyond.)
    This table implementation is my second attempt 
    at making convenient, easy to use, simple hash table.

    Table has first slot for garbage or undefined values.
    Comma expressions are abused as much as possible to 
    make it "convenient", not really a problem for 
    compatibility between C standards tho!
*/

typedef char hc_table_byte;
typedef void hc_table_unused;
typedef struct {size_t length; const char* ptr; } hc_TableKey;

#define hc_Table(T) \
    struct {\
        void        *memory;\
        T           *items;\
        hc_TableKey *keys;\
        size_t  memory_size, count, capacity;\
        size_t  last_op_status_code;\
        size_t (*hash1) (const char*, size_t);\
        size_t (*hash2) (const char*, size_t);\
    }

typedef struct {
    void            *memory_block_to_everything; /*so far unsused.*/
    hc_table_byte   *items;
    hc_TableKey     *keys;
    size_t  memory_size, count, capacity;
    size_t  last_op_status_code;
    size_t (*hash1) (const char*, size_t);
    size_t (*hash2) (const char*, size_t);
} __hc_Table;

typedef char hc_table_flags; 
enum {
    HC_TABLE_FLAGS_ASSERT_KEY_EXISTS    = (1<<0),
    HC_TABLE_FLAGS_ACT_AS_INSERT        = (1<<1),
    /*  TBD   */
};

/*TODO: resize the map.*/

#define hc_table_type_info(T)   ((void*) (T)), (sizeof((T)->items[0]))
#define hc_table_sized_value(V) (sizeof(V)), (V)
#define hc_table_sized_key(K)   (K), (strlen(K))

#define hc_table_get(table, key)\
    ((table)->items[hc_table_query(hc_table_type_info(table), key)])

#define hc_table_insert(table, key, value)\
    (hc_table_get(table, key) = value)

/* hash functions: 
   https://softwareengineering.stackexchange.com/questions/49550/which-hashing-algorithm-is-best-for-uniqueness-and-speed#145633
   */
size_t djb2    (const char* ptr, size_t size);
size_t fnv1a   (const char* ptr, size_t size);

size_t hc_table_query(void* table, size_t typesize, const char* key);
size_t hc_table_query_ex(void* table, size_t typesize,
        const char* key, size_t length, 
        hc_table_flags config);


#ifndef __HCH_TABLE_HEADER_ONLY

size_t djb2(const char* str, size_t size) {
    size_t i, h = 5381, shift = 33;
    for (i = 0; i < size; i++) 
        h = ((h << shift) + h) + str[i];
    return h;
}

size_t fnv1a(const char* data, size_t size) {
    size_t i, h = 2166136261UL;
    for (i = 0; i < size; i++) {
        h ^= data[i];
        h *= 16777619;
    }
    return h;
}

size_t hc_table_query(void* t, size_t typesize, const char* key) {
    return hc_table_query_ex(
            t, typesize, 
            key, strlen(key), 

            0
            | HC_TABLE_FLAGS_ASSERT_KEY_EXISTS 
            | HC_TABLE_FLAGS_ACT_AS_INSERT
    );
}

/*Zero slot is considered to be invalid, anything written or read
 * from there should be considered as UB.*/
size_t hc_table_query_ex(void* t, size_t typesize, 
                        const char* key, size_t length,
                        hc_table_flags config) 
#define mks_eq(map_key)\
    ((map_key.length == length) \
     && (strncmp(map_key.ptr, key, length) == 0))
#define user_or_default_hash(hash, fallback)\
        ((hash) ? hash : fallback)
#define index_is_null(idx)      !(idx)
{

    (hc_table_unused) typesize;

    __hc_Table* table = t;
    size_t i = 0;
    size_t h1, h2, index = 0;
    size_t (*fallback_hash1) (const char*, size_t) = fnv1a;
    size_t (*fallback_hash2) (const char*, size_t) = djb2;
    size_t capacity = table->capacity;
    hc_TableKey map_key = {0};
    
    bool trigger_not_found_key  = config & HC_TABLE_FLAGS_ASSERT_KEY_EXISTS;
    bool we_are_inserting       = config & HC_TABLE_FLAGS_ACT_AS_INSERT;

    /* check if initlized */
    assert(table->keys && "Expected to have table memory initilized.");
    if(!capacity) return 0;
    assert(key && length);

    h1 = user_or_default_hash(table->hash1, fallback_hash1)
        (key, length);
    map_key = table->keys[(index = h1 % capacity)];
    
    /*  check 1 */
    /*TODO: check if count doesn't exceed capacity */
    if (we_are_inserting) {
        if (!index_is_null(index) && !map_key.ptr) 
            return table->count++, index; 
    } else {
        if (!index_is_null(index) && map_key.ptr && mks_eq(map_key)) 
            return index; 
    }

    /* check 2 (double hash) */ 
    h2 = user_or_default_hash(table->hash2, fallback_hash2)
        (key, length);
    map_key = table->keys[(index = (h1+h2) % capacity)]; 

    if (we_are_inserting) {
        if (!index_is_null(index) && !map_key.ptr) 
            return table->count++, index; 
    } else {
        if (!index_is_null(index) && map_key.ptr && mks_eq(map_key)) 
            return index; 
    }

    /*  try linear lookup  */
    for(i = 0; i < capacity; i++) {
        map_key = table->keys[(index = ((h1+h2)+i)%capacity)]; 
        if(we_are_inserting) {
            if(!map_key.ptr) 
                return table->count++, index;
        } else {
            /*   if we have a gap, this means 
            //  desired key can't be found here since
            //  if it would exist, it would be inserted in 
            //  linear fassion with current key,
            //  gap indicated end of this key lookup sequence
            //  or buggy/corrputed behaviour of the map
            */ 
            if(!map_key.ptr) break;
            
            if(index_is_null(index) && map_key.length != length) continue;
            if(mks_eq(map_key))  
                return index;
        }
    }

    assert(trigger_not_found_key);
    return 0;
#undef mks_eq
#undef index_is_null
#undef user_or_default_hash
}

#endif /*__HCH_TABLE_HEADER_ONLY */
#endif/*__HCH_TABLE*/
