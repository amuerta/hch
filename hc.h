/*
   ARENA
*/

// 
// Simple implementation of arena allocator build on top of the posix malloc
// you can replace malloc by a system specific memory allocator, like linuxe's mmap
// or window's get..memory..something..idk i dont use windows.
//

#ifndef __ARENA_H
#define __ARENA_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>


// customize block size to your desire.
#ifndef ARENA_NODE_SIZE // 2048, (4096*1024) - linux max page size
#   define ARENA_NODE_SIZE 4096 // 4kb is default linux page size. 
#endif

#define ARENA_HEADER_SIZE sizeof(ArenaNode)


typedef unsigned char arena_bitmask8;
enum {
    ARENA_RESET_SIZE   = (1 << 0),
    ARENA_RESET_MEMORY = (1 << 1),
    ARENA_FREE_NODES   = (1 << 2),
};

typedef struct ArenaNode {
    size_t              allocated;
    struct ArenaNode*   next;
    char                data[];
} ArenaNode;

typedef struct {
    size_t      totally_allocated;
    ArenaNode*  memory;
} Arena;


// use these
void*       arena_alloc     (Arena*, size_t);
void        arena_memcpy    (Arena* a, void* data, size_t size);
void        arena_reset     (Arena*, int opt);
#define     arena_put(A, I) arena_memcpy(A, &I, sizeof(I))
#define     arena_clear(A)  arena_reset(A, ARENA_RESET_SIZE | ARENA_RESET_MEMORY)
#define     arena_rewind(A) arena_reset(A, ARENA_RESET_SIZE)
#define     arena_free(A)   arena_reset(A, ARENA_RESET_SIZE | ARENA_RESET_MEMORY | ARENA_FREE_NODES)


// change this one for your specific need/enviorment/taste
ArenaNode* arena_make_node(void);



ArenaNode* arena_make_node(void) {
    return calloc(ARENA_NODE_SIZE, 1);
}

void* arena_alloc(Arena* a, size_t size) {
    assert(ARENA_NODE_SIZE > size);
    void* ret = 0;
    ArenaNode* tail = a->memory;
    
    if (!a->memory) {
        a->memory = arena_make_node();
        tail = a->memory;
        goto arena_alloc_goto;
    }
    while(tail->next) tail = tail->next;

arena_alloc_goto:
    if (tail->allocated + size < (ARENA_NODE_SIZE - ARENA_HEADER_SIZE)) {
        ret = tail->data + tail->allocated;
        tail->allocated += size;
    } else {
        tail->next = arena_make_node();
        tail = tail->next;
        goto arena_alloc_goto;
    }

    return ret;
}




void arena_reset(Arena* a, int opt) {
    a->totally_allocated = 0;
    ArenaNode* node = a->memory;
    
    while(node) {
        ArenaNode* to_free = node;
        if (opt & ARENA_RESET_SIZE)
            node->allocated = 0;
        if (opt & ARENA_RESET_MEMORY)
            memset(node->data, 0, ARENA_NODE_SIZE - ARENA_HEADER_SIZE);

        node = node->next;
        if (opt & ARENA_FREE_NODES) 
            free(to_free);
    }
}

void arena_memcpy(Arena* a, void* data, size_t size) {
    void* cell = arena_alloc(a, size);
    assert(cell && "Unexprected null, failed to allocate");
    memcpy(cell, data, size);
}


#endif//__ARENA_H
//
// ARGS - argument parsing
//


// TODO: add flags that don't need '--'
// TODO: add support '--flag=<VALUE>'


#ifndef __HCH_ARGS_H
#define __HCH_ARGS_H

#include <stdlib.h>  
#include <stdio.h>   
#include <string.h>  
#include <assert.h>  
#include <stdbool.h> 

typedef struct {
    char**          items;
    int             count;
} ArgsSlice;

bool arg_str_is_flag(const char* str) {
    return str && strlen(str) >= 2 && (strncmp(str,"--", 2)==0 || *str == '-') ;
}

int arg_flag(ArgsSlice args, const char* flag) {
    assert(flag);
    for(int i = 0; i < args.count; i++) {
        const char* s = args.items[i];
        if (arg_str_is_flag(s)) 
            if (*s == '-' || (*s == '-' && strcmp((s+2), flag)==0)) 
                return i;
    }
    return 0;
}

int arg_list(ArgsSlice args, const char* flag, ArgsSlice* out) {
    int b = 0;
    if((b = arg_flag(args,flag))) {
        if (b+1 < args.count)   out->items = args.items + b+1;
        else                    out->items = 0;

        for(int i = b+1; i < args.count; i++) {
            if(arg_str_is_flag(args.items[i])) break;
                out->count++;
        }
    }
    return b;
}

#endif//__HCH_ARGS_H
/*
    DYNAMIC ARRAY
*/


#ifndef __DA_H
#define __DA_H

#include <stdlib.h>  
#include <stdio.h>   
#include <string.h>  
#include <assert.h>  

// it is just more useful then a standalone implementation 
// of dynamic array with void*
#ifndef da_append // if no da_append
                  // implement it

// This dynamic array is a trick i have seen @Tsoding use,
// and its (probably) the simplest way of doing dynamic array i used.
// 
// It's sort of robust and convinient to use.
// HOWEVER it's just a macro, which can be an issue when trying to port 
// projects/libraries to other languages.
//
// My solution:
//  Have api that either calls macros, or actual functions.
//  You can change which one to use with simple toggle.
//  Behaviour remains the same in both cases.
//
//  This way you can use da_append outside of C, 
//  its pretty dirty with `void*` but you can create interfaces
//  in pretty much any other language other than C, or use macros of any kind.
#define DA_GROW_FACTOR 2

#ifdef DA_LINKABLE
#   define da_append(DA, VAR) __da_append_generic(&((DA)->items), &(VAR), sizeof((VAR)));
#else
#   define da_append(DA, VAR) __da_append_macro((DA), (VAR));
#endif

// generic "interface"
typedef struct {
    void* items;
    size_t count, capacity, typesize;
} DaGeneric;

#define __da_append_macro(DA, VAR) do { \
    if ((DA)->capacity == 0) {(DA)->capacity = 32; (DA)->items = calloc(32,sizeof(*(DA)->items));}\
    if ((DA)->count >= (DA)->capacity) {\
        (DA)->capacity *= DA_GROW_FACTOR;\
        (DA)->items = realloc((DA)->items,sizeof(*(DA)->items) * (DA)->capacity);\
    }\
    (DA)->items[((DA)->count)++] = VAR;\
} while(0);

void __da_append_generic(void* da_ptr, void* varptr, size_t size) {
    DaGeneric* da = da_ptr;
    if (da->capacity == 0) {
        da->capacity = 32; 
        da->items = calloc(32,sizeof(*da->items));
        da->typesize = size;
    }
    assert(size == da->typesize);
    if (da->count >= da->capacity) {
        da->capacity *= DA_GROW_FACTOR;
        da->items = realloc(da->items,size * da->capacity);
    }
    memcpy(da->items + (da->count * size), varptr, size);
    da->count++;
}



//
// QoL (im lazy)
//
#define DA_HEADER(T) \
        T* items;\
        size_t count, capacity, typesize;

#define da_loop(DA,I) for(size_t I = 0; I < DA.count; I++)
#define da_get(DA) ((DA).items)

#endif// da_append 
#endif// __DA_H
#ifndef __LINK_H
#define __LINK_H

#include <stdlib.h>  
#include <stdio.h>   
#include <string.h>  
#include <assert.h>  

//
// Link appendage (Linked List)
//
#ifndef li_append

#define li_append(L, I) do {\
    if(!(L)) {(L) = (I); (L)->tail = (L);}\
    else {\
        void* __list_time__ = (I);\
        (L)->tail->next = __list_item__;\
        (L)->tail = __list_item__;\
    }\
}while(0)

#define li_next(LI) ((LI)->next)

#define li_foreach(LI, T, I, ...) do {\
   T* __next__ = (LI);\
   T* __prev__ = (LI); (void)__prev__; (void)__next__;\
   while(next) {\
       T* I = __next__;\
       {__VA_ARGS__}\
       prev = __next__;\
       next = __next__->next;\
   }\
} while(0)

#define li_defer(LI, T, ...) do {\
   T* next = (LI);\
   T* prev = (LI);\
   while(next) {\
       prev = next;\
       next = next->next;\
       {__VA_ARGS__}\
       prev = 0;\
   } (LI) = 0;\
} while(0)

#endif//li_append
#endif//__LINK_H
#ifndef __HCH_MAP_H
#define __HCH_MAP_H

#include <stdlib.h>  
#include <stdio.h>   
#include <string.h>  
#include <assert.h>  
#include <stdbool.h> 

#ifndef MAPS_DEFAULT_INIT_SIZE
#   define MAPS_DEFAULT_INIT_SIZE 2048
#endif

// Map uses slice instead of cstring
// for obvious reasons...
typedef struct {
    const char* items;
    size_t      count;
} MapKeySlice;

typedef struct {
    MapKeySlice*    keys;
    size_t count, capacity;
} Map;


// hash functions: 
// https://softwareengineering.stackexchange.com/questions/49550/which-hashing-algorithm-is-best-for-uniqueness-and-speed#145633
static inline unsigned long djb2    (const char* str, size_t size, char shift);
static inline unsigned long fnv1a   (const char* data, size_t size);

// Key
MapKeySlice map_slice(const char* str, size_t count);
MapKeySlice map_key(const char* str);

// map(String)
static inline bool  map_key_is_ok   (long int index);
float               map_load        (Map  m); // in range from 0 to 1
long int            map_query       (Map  m, MapKeySlice string);
long int            map_reserve     (Map* m, MapKeySlice string);
void                map_clear       (Map* m);

#define maps_get    (M, S)                      maps_query   (M, map_key(S))
#define maps_put    (M, S)                      maps_reserve (M, map_key(S))
//      maps_resize (m, new_size, {CODE BLOCK}) /*macro*/

static inline unsigned long djb2(const char* str, size_t size, char shift) {
    unsigned long h = 5381;
    for (size_t i = 0; i < size; i++) 
        h = ((h << shift) + h) + str[i];
    return h;
}

static inline unsigned long fnv1a(const char* data, size_t size) {
    unsigned long h = 2166136261UL;
    for (size_t i = 0; i < size; i++) {
        h ^= data[i];
        h *= 16777619;
    }
    return h;
}

MapKeySlice map_key(const char* str) {
    MapKeySlice s = {.items=str, .count=strlen(str)};
    return s;
}

MapKeySlice map_slice(const char* str, size_t count) {
    MapKeySlice s = {.items=str, .count=count};
    return s;
}

static inline bool  map_key_is_ok   (long int index) {
    return index >= 0;
}

float map_load(Map m) {
    assert(m.capacity);
    return (m.count == 0) ? 0 : m.count/m.capacity;
}

Map map_alloc(Map* m, size_t cap) {
    static Map local_map;

    if(!m) m = &local_map;
    
    if (cap == 0) m->capacity = MAPS_DEFAULT_INIT_SIZE;
    else m->capacity = cap;

    m->keys = calloc(m->capacity, sizeof(*m->keys));
    return *m;
}

// the idea is that for each map you implement, you call this 
// generic thing to implement map resize
//
// check example for details: ./examples/maps.c

// for now i don't handle undersizing the map.
#define map_resize(m, new_size, ...) do {\
    if (new_size <= (m)->capacity) break; \
    Map new_map = map_alloc(0, new_size);\
    for(size_t i = 0; i < (m)->capacity; i++) {\
        MapKeySlice key = (m)->keys[i];\
        if (!key.items || !key.count) continue;\
        long int oldid = (long int) i;\
        long int newid = map_reserve(&new_map, key);\
        assert(newid != -1 && "Should always be sucessful");\
        new_map.keys[newid] = key;\
        {__VA_ARGS__}\
    }\
    free((m)->keys);\
    m->keys = new_map.keys;\
    m->capacity = new_map.capacity;\
} while(0)

void map_clear(Map* m) {
    m->count = 0;
    m->capacity = 0;
    free(m->keys);
}


long int map_reserve(Map* m, MapKeySlice string) {
#define     hf1(c, size) (fnv1a(c, size))
#define     hf2(c, size) (djb2(c, size, 33))
    
    assert(m->keys && "call map_alloc() first");

    unsigned long 
        h1 = hf1(string.items, string.count),
        h2,
        index = -1
    ;
    MapKeySlice key = m->keys[(index = (h1 % m->capacity))];
    const size_t cap = m->capacity;

    // collision
    if (key.items) {
        h2 = hf2(string.items, string.count);
        // we hit again
        if (m->keys[(index = (h1+h2)% m->capacity)].items) {
            // "fuck it - iterative approach"
            for(size_t i = 0; i < m->capacity; i++) {
                if (!m->keys[(index = ((h1+h2)+i)%cap)].items) 
                    goto end;
            }
            return -1;
        } 
    } 

end:
    m->count++;
    if ((long int)index >= 0) m->keys[index] = string;
    return (long int) index;

#undef  hf1
#undef  hf2
}

long int map_query(Map m, MapKeySlice string) {
 
#define hf1(c, size) (fnv1a(c, size))
#define hf2(c, size) (djb2(c, size, 33))
#define mks_eq(k,s) (strncmp(k.items, s.items, s.count) == 0)

    // initlized
    if(!m.capacity || !m.keys) return -1;
    //  good practice
    assert(string.items && string.count);


    unsigned long
        h1 = hf1(string.items, string.count),
        h2,
        index = -1
    ;

    size_t len = string.count;

    // check 1
    size_t cap = m.capacity;
    MapKeySlice key = m.keys[(index = h1 % m.capacity)];

    if (key.items && key.count == len && mks_eq(key, string)) 
        return (long int)index;

    // check 2 (double hash)
    h2 = hf2(string.items, string.count);
    key = m.keys[(index = (h1+h2)% cap)]; 

    if (key.items && key.count == len && mks_eq(key, string))  
        return (long int)index;

    //  try linear lookup  
    for(size_t i = 0; i < m.capacity; i++) {
        key = m.keys[(index = ((h1+h2)+i)%cap)]; 
        if(!key.items) break; // if we have a gap, this means 
                              // desired key can't be found here since
                              // if it would exist, it would be inserted in 
                              // linear fassion with current key,
                              // gap indicated end of this key lookup sequence
                              // or buggy/corrputed behaviour of the map
        if(key.count != len) continue;
        if (mks_eq(key, string))  
            return (long int)index;
    }

    return -1;

#undef mks_eq
#undef hf1
#undef hf2
}


#endif//__HCH_MAP_H


//
// POOL datastructure
//

#include <stdlib.h>  
#include <stdio.h>   
#include <string.h>  
#include <assert.h>  

// TODO:? make it use not void* but user defined union/struct?

#ifndef __HCH_POOL_H
#define __HCH_POOL_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>


// TODO: use inline __asm__(int3) to have a proper breakpoint 
// instead of this old funny hack
// cause segmentaion fault to be able to run gdb on breakpoint
#ifdef DEBUG_SEGFAULT_ON_ASSERT
#   define FAULT_TRIGGER \
        *((int*)0) = 1 
#endif

#ifndef FAULT_TRIGGER
#define FAULT_TRIGGER // does nothing 
#endif

#ifndef hch_assert
#define hch_assert(COND,...) \
    do { if (!(COND)) { \
        fprintf(stderr,"Assertion at [%s:%s:%d]: ",__FILE__,__func__,__LINE__); \
        fprintf(stderr,__VA_ARGS__); \
        fprintf(stderr,"\n"); \
        FAULT_TRIGGER;      \
        fprintf(stderr, "NOTE: you can define FAULT_TRIGGER to enable gdb breakpoint\n");\
        exit(1);            \
    }} while(0)
#endif

typedef size_t              index_t;
typedef unsigned char       bitmask8;

#define INDEX_INVALID ((size_t)-1)

typedef enum {
	PoolState_allocated = 1,
} PoolState;

typedef struct {
    size_t      typesize;
    char*       type;

    void*       data;
    index_t*      free_indexes;
    

    void        (*destructor) (void*);
    size_t      capacity;
    size_t      count;
    size_t      free_count;
    size_t      max_size;
} Pool;

#ifndef POOL_MALLOC
#   define POOL_MALLOC(S) malloc(S)
#endif

#ifndef POOL_FREE
#   define POOL_FREE(P) free(P)
#endif

#ifndef POOL_REALLOC
#   define POOL_REALLOC(P,S) realloc(P,S)
#endif


#ifndef IGNORE_RETURN
#   define IGNORE_RETURN (void)
#endif

#define __POOL_typestring(T) #T

#ifndef POOL_DEFAULT_CAPACITY
#   define POOL_DEFAULT_CAPACITY 32
#endif

#define POOL_ITEM_POINTER(p,index) \
    p->data + index * (p->typesize + sizeof(bitmask8));



//      //
/* API  */
//      //

#define     pool_new(T)                         pool__init(NULL, POOL_DEFAULT_CAPACITY, sizeof(T), __POOL_typestring(T))
#define     pool_init(P,T,S)                    IGNORE_RETURN pool__init(P, S, sizeof(T), __POOL_typestring(T))
void        pool_resize(Pool* p, size_t newsize);
index_t     pool_reserve(Pool* p);
void        pool_release(Pool* p, index_t i);
void*       pool_refer(Pool* p, index_t i);


Pool pool__init(Pool* self, size_t capacity, size_t typesize, char* type) {
    const size_t data_sz_bytes = capacity * ( sizeof(bitmask8) + typesize );
    const size_t idxs_sz_bytes = capacity * sizeof(index_t);

    Pool new = {
        .type = type,
        .typesize = typesize,
        .capacity = capacity,
        .count = 0,
        .free_count = 0,
        .max_size = 0,

        // alloc memory,
        .data = POOL_MALLOC(data_sz_bytes),
        .free_indexes = POOL_MALLOC(idxs_sz_bytes),
    };

    if (!self) {
        return new;
    } 

    memcpy(self, &new, sizeof(new));
    return *self;
}

void pool_free(Pool* p) {
    free(p->data);
    free(p->free_indexes);
    memset(p,0,sizeof(*p));
}

void* pool_refer(Pool* p, index_t i) {
    hch_assert(p, "Expected to have valid pointer got NULL");
    hch_assert(p->data, "Expected to have valid data pointer initilized");
    void* ptr = POOL_ITEM_POINTER(p,i);
    bitmask8 state = *((bitmask8*)ptr);
    return (state) ? ptr : NULL;
}

void pool_resize(Pool* p, size_t newsize) {
    const size_t newsize_bytes = newsize * (sizeof(bitmask8) + p->typesize);
    const size_t newsize_indexes_bytes = newsize * sizeof(index_t);

    if (p->capacity == newsize) 
        return;
    else if (p->capacity > newsize) {
        // TODO:
        // impl destructor
    } 
 
    p->capacity     = newsize;
    p->data         = POOL_REALLOC(p->data,         newsize_bytes           );
    p->free_indexes = POOL_REALLOC(p->free_indexes, newsize_indexes_bytes   );
}

#define pool_append(P, VAR) \
    pool__append(P, &(VAR), sizeof(VAR))

index_t pool__append(Pool* p, void* data, size_t typesize) {
    index_t i = pool_reserve(p);
    if (i == INDEX_INVALID) 
        return INDEX_INVALID;

    hch_assert(typesize == p->typesize, 
            "Expected to have a type that equal or less than pool typesize");
    memcpy(pool_refer(p,i),data,typesize);
    return i;
}

index_t pool_reserve(Pool* p) {
    index_t index = INDEX_INVALID;
    if (p->free_count > 0) {
        index = p->free_indexes[p->free_count-1];
        p->free_count--;
    } else {
        index = p->count;
    }
    //debug("Reserved id: %u\n",ptr);
    hch_assert(index != INDEX_INVALID, "Failed to reserve entity");
    hch_assert(p->count < p->capacity, "Attempt to buffer overflow");

    bitmask8* state = POOL_ITEM_POINTER(p, index);
    *state |= PoolState_allocated;

    p->count++;
    if (p->count > p->max_size) {
        p->max_size = p->count;
    }
    return index;
}

void  pool_release(Pool* p, index_t i) {
    if (p->count==0)
        return;

    hch_assert(i < p->capacity, "Attempt to access Out of Bounds");
    bitmask8* state = POOL_ITEM_POINTER(p,i);

    if (!(*state & PoolState_allocated))
        return;
 
    p->free_indexes[p->free_count] = i;
    p->free_count++;

    *state ^= PoolState_allocated;
    p->count--;
}

#endif //__HCH_POOL_H


//
// TYPES
//

#ifndef __HCH_TYPES_H
#define __HCH_TYPES_H


#include <stdio.h>   
#include <string.h>  
#include <assert.h>  
#include <stdbool.h> 
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>

typedef int8_t              i8;
typedef int16_t             i16;
typedef int32_t             i32;
typedef int64_t             i64;
typedef intmax_t            isize;

typedef int8_t              s8;
typedef int16_t             s16;
typedef int32_t             s32;
typedef int64_t             s64;
typedef intmax_t            ssize;

typedef uint8_t             u8;
typedef uint16_t            u16;
typedef uint32_t            u32;
typedef uint64_t            u64;
typedef size_t              usize;

typedef float               f32;
typedef double              f64;
                              
typedef const char*         istr;
typedef char*               mstr;

typedef size_t              index_t;
typedef unsigned char       bitmask8;
typedef unsigned short      bitmask16;
typedef unsigned int        bitmask32;
typedef uint64_t            bitmask64;

#endif // __HCH_TYPES_H

//
// MACROS
//

#define arrlen(a)           (sizeof(a)/sizeof(a[0]))
#define cast(v, T)          ((T)v)
#define transmute(v, T)     *((T*)&(v))
#define zeroed(v)           memset(&(v), 0, sizeof(v))
#define unused(v)           ((void) (v))

#ifndef max
	#define max(A,B) (A > B) ? A : B
#endif

#ifndef min
	#define min(A,B) (A < B) ? A : B
#endif

#ifndef loop
	#define loop(I,N) for(size_t I = 0; I < (N); I++)
#endif

#ifndef loopt
	#define loopt(TI,N) for(TI = 0; I < (N); I++)
#endif

// cause segmentaion fault to be able to run gdb on breakpoint
#ifdef DEBUG_SEGFAULT_ON_ASSERT
#   define FAULT_TRIGGER \
        *((int*)0) = 1 
#endif

#ifndef FAULT_TRIGGER
#define FAULT_TRIGGER // does nothing 
#endif

#ifndef hch_assert
#define hch_assert(COND,...) \
    do { if (!(COND)) { \
        fprintf(stderr,"Assertion at [%s:%s:%d]: ",__FILE__,__func__,__LINE__); \
        fprintf(stderr,__VA_ARGS__); \
        fprintf(stderr,"\n"); \
        FAULT_TRIGGER;      \
        fprintf(stderr, "NOTE: you can define FAULT_TRIGGER to enable gdb breakpoint\n");\
        exit(1);            \
    }} while(0)
#endif
/*
   String Builder (Nob style)
*/

#include <stdlib.h>  
#include <stdio.h>   
#include <string.h>  
#include <assert.h>  
#include <stdbool.h> 

#ifndef __HCH_SB_H
#define __HCH_SB_H

typedef struct {
    // transmutable -> DA , string
    char*  items;
    size_t count, capacity;

    const char* spacer;
} StringBuilder;

#define sb_arrlit(...)          ((const char*[]) {__VA_ARGS__})
#define sb_arrlen(arr)          (sizeof(arr) / sizeof((arr)[0]))
#define sb_arrlit_len(...)      (sb_arrlen((__VA_ARGS__)))

#define sb_min(a,b) ((a) > (b))? (b) : (a)
#define sb_max(a,b) ((a) < (b))? (b) : (a)

// TODO: move this somewhere else higher up
// it can be wiedly used.
void* recalloc(void* ptr, size_t prev_size, size_t size) {
    void* new_ptr = calloc(size, 1);
    if(ptr) {
        memcpy(new_ptr, ptr, prev_size);
        free(ptr);
    }
    return new_ptr;
}

void sb__append(StringBuilder* sb, const char** items, size_t count) {
    if (!count) return;
    size_t append_size = 0;
    size_t spacer_size = 0;

    if (sb->spacer) spacer_size = strlen(sb->spacer); 

    for(size_t i = 0; i < count; i++) {
        if (!items[i]) continue;
        append_size += strlen(items[i]);
        if (i != count - 1 || i) 
            append_size += spacer_size;
    }
    // null terminator
    if(append_size) append_size++;

    if (!sb->items || !sb->capacity) {
        sb->capacity = 32;
        sb->items = calloc(32, 1);
    }
    if (sb->count + append_size >= sb->capacity) {
        size_t new_size = sb_max(sb->capacity*2, sb->capacity + append_size);
        sb->items = recalloc(sb->items, sb->capacity, new_size);
        sb->capacity = new_size;
    }

    for(size_t i = 0; i < count; i++)  {
        if (!items[i]) continue;
        size_t len = strlen(items[i]);
        if(items[i]) {
            memcpy(sb->items + sb->count, items[i], len);
            sb->count += len;
            
            if (spacer_size && (i != count-1)) {
                memcpy(sb->items + sb->count, sb->spacer, spacer_size);
                sb->count += spacer_size;
            }

        }
    }
}

void sb_appendf(StringBuilder* sb, const char* fmt, ...) {
    va_list args, args_len;
    va_start(args, fmt);
    va_copy(args_len, args);
    size_t size = vsnprintf(0,0,fmt,args_len);
    va_end(args_len);

    const char* temp = calloc(size+1,1);
    vsnprintf((char*)temp, size+1, fmt, args);
    sb__append(sb, &temp, 1);
    free((void*)temp);
    
    va_end(args);
}

void sb_clear(StringBuilder* sb) {
    memset(sb->items, 0, sb->count);
    sb->count = 0;
}

#define sb_append(sb, ...) \
    sb__append(\
            sb,\
            sb_arrlit(__VA_ARGS__),\
            sb_arrlen(sb_arrlit(__VA_ARGS__)))\

#endif//__HCH_SB_H
#ifndef __HCH_STRING_H
#define __HCH_STRING_H

#include <stdlib.h>  
#include <stdio.h>   
#include <string.h>  
#include <assert.h>  
#include <stdbool.h> 

#define STR_MAX_REFER_SIZE 256
#define FULL_LENGTH 0
#define STR_NOPATTERN -1

// Dynamic array - "da", can be
// transmuted into String directly
typedef struct {
	char* 	ptr;
	size_t 	len;
	size_t  cap;
} String;


String str_prealloc(size_t cap);
	// allocates memory for cap 
	// amount of characters
	// in empty string of len 0
String 	str_from_cstr(char* cstr, size_t len); 
	// creates a string view from cstr, creates a copy of cstr,
	// allocates memory!
String 	str_move_cstr(char* cstr);
	// creates a string view without ownership of cstr,
	// DOES NOT allocate memory!
String 	str_create(char* cstr);
	// shorten version of str_from_cstr(char*,size_t);
	// allocated memory!
String 	str_dup(String orig);
	// creates a copy of existing String
	// allocates memory!
String 	str_substr(String orig, size_t index, size_t len);
	// creates a new string from origin begining from index
	// and finishing at index+len
	// if index or len go out of bounds
	// asserts error
String* str_split(String src, char divisor, size_t* count);
	// splits strings src by a divisor char
	// return newelly allocated buffer of
	// strings, writes thier amount to ~count~ ptr
	// allocated memory!
void 	str_append_chars(String *s,char* chars);
	// appends any N of chars to the end of 
	// the String s
void 	str_append_char(String *s, char ch);
	// appends single char to String s
void 	str_reverse(String* s);
	// reverses the string characters
bool 	str_begins_with(String src, String pat);
	// returns true if pattern is the begining of source
	// asserts any of strings len is 0 or 
	// when pattern is bigger then source
bool 	str_ends_with(String src, String pat);
	// same as str_begins_with(String,String) 
	// but checks the end of the string instead
	// asserts follow the same rules as in a
	// function above
bool 	str_are_equal(String l, String r);
	// returns true if strings are equal
bool 	str_is_empty(String s);
	// returns true if s.len == 0 or when 
	// string pointer points to NULL
int 	str_has_pattern(String src, String pat);
	// iterates over string source to find 
	// substring defined in pattern,
	// returns begining index of pattern on sucess
	// -1 or STR_NOPATTERN on failure
    // NOTE: SLOW!!
bool 	str_is_integer(String s);
	// returns true if all the characters
	// are either numbers or a minus sign
bool 	str_is_float(String s);
	// same as str_is_integer(String)
	// but also checks for a dot: '.'
char* 	str_get_cstr(String s);
	// returns copied string pointer terminated by 0
	// needs to be freed after seperately

#define str_print(s) str_print_fmt(s,0,0)
void	str_print_fmt(String s, char* prefix, char* postfix);

char*	str_get_temp(String s);
	// creates a static string of STR_STATIC_SIZE
	// used for printing and <string.h> functions
	// in order to modify content, create another
	// buffer/string, and copy stuff in there.
void 	str_clear(String* s);
	// resets string contents
	// and length 
void 	str_free(String* s);
	// frees the memory
	// resets every field to 0
	
// TODO: find quicker comparison for sequence of characters
//
#if 0
bool	str_memcmp(char* src, char* cmp, size_t pos, size_t len);
	// compares
bool	str_memncmp(char* src, char* cmp, size_t block_sizes[2], size_t pos, size_t len);
	// compares a block of memory in src at index pos,
#endif

//
// IMPLEMENTATION
//

String str_prealloc(size_t cap) {
	return (String) {
		.cap = cap,
		.len = 0,
		.ptr = calloc(cap,sizeof(char)),
	};
}

String str_from_cstr(char* cstr, size_t len) {
	
	size_t prealloc_count;
	assert(strlen(cstr)>=len && 
			"String length cannot be greater"
			" than provided cstr len"		 );

	// if len set as 0, use entire string length
	if (len == 0)
		len = strlen(cstr);
	
	prealloc_count = len;
#ifdef STR_PREALLOC_BYTES
	prealloc_count = max(STR_PREALLOC_BYTES,len);
#endif

	String s = {0};
	s.len = len;
	s.cap = prealloc_count;
	s.ptr = calloc(prealloc_count,sizeof(char));

	if (!s.ptr) assert(false && 
			"Failed to allocate"
			"memory with calloc(n,s)"
		);

	memcpy(s.ptr,cstr,len);
	return s;
}

String str_create(char* cstr) {
	return str_from_cstr(cstr,FULL_LENGTH);
}

String str_move_cstr(char* cstr) {
	size_t len = strlen(cstr);
	String s = {
		.len = len,
		.ptr = cstr,
		.cap = len,
	};
	return s;
}

String str_dup(String s) {
	String dup = {0};
	
	if (str_is_empty(s))
		return dup;

	assert(s.len != 0 && 
			"String is not reset, but has 0 length");

	dup.ptr = calloc(s.len,	sizeof(char));
	dup.len = s.len;
	dup.cap = s.len;

	if (!dup.ptr)
		assert(false && 
				"Failed to allocate"
				"memory with calloc(n,s)");
	memcpy(dup.ptr,s.ptr,s.len);
	return dup;
}


String str_substr(String orig, size_t index, size_t len) {
	assert(index < orig.len && 
			"Index overflows original string");
	assert(len < orig.len && 
			"Sub string length cannot be greated than origin len");
	assert(index + len <= orig.len && 
			"Slice go out of original string bounds");


	// alloca is buggy, fix-sized array is tedious
	char* buffer = calloc(len+1,sizeof(char)); 
	for(uint i = 0; i < len; i++) 
		buffer[i] = orig.ptr[index+i];
	
	String s = str_create(buffer);
	free(buffer);
	return s;
}

void str_append_chars(String *s,char* chars) {

	char* new_ptr 		= 0;
	size_t chars_len 	= strlen(chars);
	size_t new_len   	= s->len + chars_len;
	size_t realloc_size = s->len == 0 ?
		 1 : s->len * 2;

	if (chars_len == 0)
		return;

	if (str_is_empty(*s)) {
		*s = str_create(chars);
		return;
	}

	if (s->len + strlen(chars) > s->cap) {
		 new_ptr = reallocarray(s->ptr,
				realloc_size,
				sizeof(char));
		 s->cap = realloc_size;
	}

	if (!new_ptr)
		assert(false && 
				"failed to reallocate memory"
				"using reallocarray(ptr,nmemb,size)"
		);

	// [ j i m ]
	// [ j i m m y] <- new_len 3+2
	//   0 1 2
	//         ^  for (i,i<new_len;i++)
	//         |          ~~~~~~~~(4)
	//         len
	size_t diff = new_len - s->len;
	for(uint i = 0; i < diff; i++)
		new_ptr[i+diff] = chars[i];
			//  0 + 2 + 1 = 3
			//  1 + 2 + 1 = 4
	s->ptr = new_ptr;
	s->len = new_len;
}


void str_append_char(String *s, char ch) {
	size_t new_len   = s->len + 1;
	char* new_ptr = s->ptr;
	size_t realloc_size = s->len == 0 ?
		 1 : s->len * 2;
	
	if (str_is_empty(*s)) {
		char one_char_cstr[2] = { ch, 0 };
		*s = str_create(one_char_cstr);
		return;
	}
	if (new_len > s->cap) {
		new_ptr = reallocarray(s->ptr,
				realloc_size,
				sizeof(char));
		s->cap = realloc_size;
	}

	if (!new_ptr)
		assert(false && 
				"failed to reallocate memory"
				"using reallocarray(ptr,nmemb,size)"
		);
	
	new_ptr[s->len] = ch;

	s->ptr = new_ptr;
	s->len = new_len;
}


void str_reverse(String* s) {
	if (str_is_empty(*s))
		return;

	char* ptr_cpy = calloc(s->len,	sizeof(char));
	if (!ptr_cpy)
		assert(false && 
				"Failed to allocate"
				"memory with calloc(n,s)");
	memcpy(ptr_cpy, s->ptr, s->len);
	// [ h i ! ] : len 3
	//   i i i
	//   0 1 2
	//     ^ ^
	//	   | (end) = (len - 1)
	//	   |
	//	   +-> (end) - i

	for(uint i = 0; i < s->len; i++) {
		size_t reverse = (s->len-1) - i;
		s->ptr[i] = ptr_cpy[reverse];
	}
	free(ptr_cpy);
}

bool str_begins_with(String src, String pat) {
	assert(src.len != 0 && pat.len != 0 && 
			"Source and pattern do not allow length of 0");
	assert(src.len > pat.len && 
			"Pattern string cannot be "
			"equal or bigger to Source!"
			"Check str_are_equal(String,String)"
			);

	bool equal = true;
	for (uint i = 0; i < pat.len; i++)
		equal = equal && (src.ptr[i] == pat.ptr[i]);
	return equal;
}

bool str_ends_with(String src, String pat) {
	assert(src.len != 0 && pat.len != 0 && 
			"Source and pattern do not allow length of 0");
	assert(src.len > pat.len && 
			"Pattern string cannot be "
			"equal or bigger to Source!"
			"Check str_are_equal(String,String)"
			);

	bool equal = true;
	size_t offset = src.len - pat.len;
	for (uint i = (pat.len - 1); i > 0; i--) {
		equal = equal && (src.ptr[i+offset] == pat.ptr[i]);
		
	}
	return equal;
}

bool str_are_equal(String l, String r) {
	bool equal = true;
	if(l.len == r.len) {
		for (uint i = 0; i < l.len; i++)
			equal = equal && (l.ptr[i] == r.ptr[i]);
	} 
	else 
		return false;
	return equal;
}

bool str_is_empty(String s) {
	return (s.cap == 0 || s.ptr == NULL);
}

int str_has_pattern(String src, String pat) {
	assert(src.len != 0 && pat.len != 0 && 
			"Source and pattern do not allow length of 0");

	assert(src.len > pat.len && 
			"Pattern string cannot be "
			"equal or bigger to Source!"
			"Check str_are_equal(String,String)"
			);

	// src: [ h e l l o ! ] : len 6
	// pat: [ o ! ]  		: len 2
	
	//   0 1 2 3 4
	//   | | | | |
	//   v v v v v
	// [ h e l l o ! ]
	//         [ o ! ]
	//  last index to check is inclusive src.len - pat.len

	size_t diff = src.len - pat.len; 

	if (pat.len == 1){
		for(uint i = 0; i < src.len; i++) {
			if (src.ptr[i]==pat.ptr[0])
				return true;
		}
	}
	else
		for(uint src_i = 0; src_i <= diff; src_i++) {
			bool equal = true;

			for(uint c = 0; c < pat.len; c++)
				equal = equal && (src.ptr[src_i+c]==pat.ptr[c]);

			if (equal)
				return src_i;
		}
	return STR_NOPATTERN;
}

bool str_is_integer(String s) {
	bool is_a_num = true;
	for(uint i = 0; i < s.len; i++)
		is_a_num = is_a_num && (
			(s.ptr[i] >= '0' &&  s.ptr[i] <= '9')
			||	s.ptr[0] == '-'
		);
	return is_a_num;
}

bool str_is_float(String s) {
	bool is_a_num = true;
	for(uint i = 0; i < s.len; i++)
		is_a_num = is_a_num && (
			(s.ptr[i] >= '0' &&  s.ptr[i] <= '9')
			||	s.ptr[0] == '-' 
			||	s.ptr[i] == '.'
		);
	return is_a_num;
}


char* str_get_cstr(String s) {
	char* temp = calloc( (s.len+1)	,	sizeof(char));
	for(uint i = 0; i < s.len+1; i++)
		temp[i] = 0;
	memcpy(temp,s.ptr,s.len);
	return temp;
}

char* str_refer(String s) {
	static char buffer[STR_MAX_REFER_SIZE];
	memset(buffer,0,STR_MAX_REFER_SIZE);
	size_t len = (s.len < STR_MAX_REFER_SIZE - 1) ? 
		s.len : STR_MAX_REFER_SIZE - 1;
	strncpy(buffer,s.ptr,len);
	return buffer;
}

// TODO: increase performance?
String* str_split(String src, char divisor, size_t* count) {
	assert(src.len > 0 && "source shouldn't be empty");
	assert(count && "count shouldn't be NULL");
	String  item = str_prealloc(32);
	String* items = 0;

	loop(i, src.len) {
		bool slice_eq = src.ptr[i] == divisor;
		bool trail_str =  ( !str_is_empty(item) && i == src.len-1);

		if ( slice_eq || trail_str) {
			if (src.len-1 == i) {
				str_append_char(&item,src.ptr[i]);
			}

			const size_t newlen = ((*count)+1) * sizeof(String);
			items = realloc(items, newlen);
			items[*count] = str_dup(item);
			str_clear(&item);
			(*count)++;
		} else {
			str_append_char(&item,src.ptr[i]);
		}

	}

	str_free(&item);
	return items;
}


void str_print_fmt(String s, char* pref, char* pofx) {
	if (pref) printf("%s",pref);
	loop(i,s.len) {
		printf("%c",s.ptr[i]);
	}
	if (pofx) printf("%s",pofx);
}

void str_clear(String* s) {
	s->len = 0;
	memset(s->ptr,0,s->cap);
}

void str_free(String* s) {
	s->len = 0;
	s->cap = 0;
	if (s->ptr) 
		free(s->ptr);
	s->ptr = NULL;
}


#endif // __HCH_STRING_H
