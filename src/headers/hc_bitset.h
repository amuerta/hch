#ifndef __HC_BITSET_H
#define __HC_BITSET_H

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#ifndef __ALLOCATOR_INTERFACE
#   error "type Bitset requires allocator interface to work!"
#endif
#define BITSET_CELL_SIZE (sizeof(size_t) * 8)

#include <stdint.h>

typedef uint32_t    Bitmask32;
typedef uint64_t    Bitmask64;

#   define bitmask_check(N, M)               ((N) & (M))
#   define bitmask_toggle(N, M)              ((N) ^ (M))
#   define bitmask_set(N, M)                 ((N) | (M))
#   define bitmask_clear(N, M)               ((N) & (~(M)))
#   define bitmask_get_chunk(m, off, size)   __bm_get_chunk((m),(off),(sz))
uint64_t __bm_get_chunk(uint64_t mask, unsigned char offset, unsigned char size);
/*Either a single `size_t` (64 bit) set
 * Or more under a array to multiple `size_t` 's. */
typedef struct {
    size_t capacity;
    union {
        size_t   *array;
        size_t   single;
    } item;
} BitSet;


void bitset_upsize(Allocator allocator, BitSet* set, size_t id);
void bitset_drop(Allocator allocator, BitSet *set);
bool bitset_set(BitSet* set, unsigned long id);

/*TODO: check if pointer is not within bitset when resizing (array_inplace) */
void bitset_upsize(Allocator allocator, BitSet* set, size_t id) {
    size_t new_capacity, *new_array;
    bool bitset_too_small_for_allocations = 
        (set->capacity <= BITSET_CELL_SIZE && id < BITSET_CELL_SIZE);
    if(set->capacity > id || bitset_too_small_for_allocations) 
        return;
    assert(id && "Has to be greater than 0.");
    
    new_capacity    = ((size_t)ceil((float)id/(float)BITSET_CELL_SIZE)) * BITSET_CELL_SIZE;
    size_t new_size_bytes = new_capacity/sizeof(size_t);
    size_t old_size_bytes = set->capacity/sizeof(size_t);

    if(set->capacity <= BITSET_CELL_SIZE && id >= BITSET_CELL_SIZE) {
        new_array           = allocator_alloc(allocator, new_size_bytes);
        memset(new_array, 0, new_size_bytes);
        *new_array          = set->item.single;
    } 
    else {
        if(allocator.free) {
            new_array = allocator_alloc(allocator, new_size_bytes);
            memset(new_array, 0, new_size_bytes);
            memcpy(new_array, set->item.array, old_size_bytes);
            
            allocator_free(allocator, set->item.array, old_size_bytes);
        } else {
            /*leak memory intentionaly.*/
            new_array = allocator_alloc(allocator, new_size_bytes);
            memcpy(new_array, set->item.array, old_size_bytes);
        }
    }

    set->item.array  = new_array;
    set->capacity    = new_capacity;
}

void bitset_drop(Allocator allocator, BitSet *set) {
    if(set->capacity > BITSET_CELL_SIZE && allocator.free) {
        allocator_free(allocator, set->item.array, set->capacity/BITSET_CELL_SIZE);
        set->item.array = NULL;
    } else {
        set->item.single = 0;
    }
    set->capacity = 0;
}


bool bitset_set(BitSet* set, unsigned long id) {
    if(!set->capacity) set->capacity = BITSET_CELL_SIZE;
    if(id >= set->capacity) return false;
    set->item.single |= (1 << id);
}

bool bitset_get(BitSet s, unsigned long id) {
    size_t i = 0, cell_id = 0, cell = 0, relative_id = id;

    assert(!(s.capacity % BITSET_CELL_SIZE) &&
            "Bits capacity needs to be divisible by (sizeof(size_t)*8)");
    if(id >= s.capacity) return false;
    
    if(s.capacity > BITSET_CELL_SIZE) {
        cell_id = id/BITSET_CELL_SIZE;
        relative_id = (id % BITSET_CELL_SIZE);
        cell = s.item.array[cell_id];
    } else {
        cell = s.item.single;
    }

    return (cell >> relative_id) & 0x1;
}

bool bitset_clear(BitSet* set, unsigned long id) {
    if(!set->capacity) set->capacity = BITSET_CELL_SIZE;
    if(id >= set->capacity) return false;
    set->item.single &= ~(1 << id);
    return true;
}

const char* bitset_to_cstring(Allocator allocator,  BitSet s) 
#define string_push(s,c) ((*(s)++) = (c))
{
    char* string = 0, *result = 0;
    size_t size = 0;
    size_t i = 0, c = 0, cells = 0;
    if(s.capacity > BITSET_CELL_SIZE) {
        assert(!(s.capacity % BITSET_CELL_SIZE) &&
                "Bitset capacity needs to be divisible by (sizeof(size_t)*8)");
        cells  = (size_t)floor((float)s.capacity/(float)BITSET_CELL_SIZE);
        size   = cells * BITSET_CELL_SIZE + cells;
        string = allocator_alloc(allocator, size);
        result = string;
        for(c = 0; c < cells; c++) {
            for(i = 0; i < BITSET_CELL_SIZE; i++) {
                size_t index = c*BITSET_CELL_SIZE + i;
                string_push(string, bitset_get(s, index) ? '1':'0');
            }
            string_push(string, '\n');
        }
        string_push(string, '\0');
    } else {
        enum {NULL_TERM = 1};
        size = sizeof(char)*BITSET_CELL_SIZE;
        string = allocator_alloc(allocator, size + NULL_TERM);
        for(i = 0; i < size; i++) string[i] = bitset_get(s, i) ? '1':'0';
    }
    return result;
#undef string_push
}

uint64_t __bm_get_chunk(uint64_t mask, unsigned char offset, unsigned char size) {
    int select_mask = 0;
    for(int i = 0; i < size; i++) select_mask |= (1 << i);
    return (mask >> offset) & select_mask;
}

#endif/*__HC_BITSET_H*/
