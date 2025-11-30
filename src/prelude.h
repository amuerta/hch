//
// PRELUDE
//
/* stuff like:
 *  - types
 *  - common utility macros
 *  - bitmask handling
 *  - temporary allocator
 *
*/
#ifndef __HCH_PRELUDE_H
#define __HCH_PRELUDE_H


#include <stdio.h>   
#include <time.h>   
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
typedef int64_t             stime;
typedef uint64_t            utime;


//
// MACROS
//

# define hc_max(A,B)            (A > B) ? A : B
# define hc_min(A,B)            (A < B) ? A : B
# define hc_loop(I,N)           for(size_t I = 0; I < (N); I++)
# define hc_loopt(TI,N)         for(TI = 0; I < (N); I++)
# define hc_range(n, min, max)  ((n)>=(min) && (n)<=(max))
# define hc_clamp(n, min, max)  \
     ((n) < (min)) ? (min) : ((n) > (max) ? (max) : (n)) 

# define hc_arrlen(a)           (sizeof(a)/sizeof(a[0]))
# define hc_cast(v, T)          ((T)v)
# define hc_transmute(v, T)     *((T*)&(v))
# define hc_zeroed(v)           memset(&(v), 0, sizeof(v))
# define hc_unused(v)           ((void) (v))
# define hc_roptr(v)            ((const void*) v)
# define hc_cmp(l,r)            (memcmp(&(l),&(r),hc_min(sizeof(l),sizeof(r)))==0)
# define hc_BREAKPOINT()        __asm__("int3")

#ifdef HCH_STRIP_MACRO_PREFIX
# define arrlen(a)       hc_arrlen(a) 
# define cast(v, T)      hc_cast(v, T)     
# define transmute(v, T) hc_transmute(v, T)
# define zeroed(v)       hc_zeroed(v)      
# define unused(v)       hc_unused(v)      
# define roptr(v)        hc_roptr(v)       
# define cmp(l,r)        hc_cmp(l,r)       
# define BREAKPOINT()    hc_BREAKPOINT()   

# define max(A,B)           hc_max(A,B)          
# define min(A,B)           hc_min(A,B)          
# define loop(I,N)          hc_loop(I,N)         
# define loopt(TI,N)        hc_loopt(TI,N)       
# define range(n, min, max) hc_range(n, min, max)
# define clamp(n, min, max) hc_clamp(n, min, max)
#endif//HCH_STRIP_PREFIX

//
// bitmasking
//

#ifndef HCH_FULL_BITMASK_PREFIX
#   define bit_check(N, M)               ((N) & (M))
#   define bit_toggle(N, M)              ((N) ^ (M))
#   define bit_set(N, M)                 ((N) | (M))
#   define bit_clear(N, M)               ((N) & (~(M)))
#   define bit_get_chunk(m, off, size)   __bm_get_chunk((m),(off),(sz))
#else
#   define bitmask_toggle(N, M)             ((N) ^ (M))
#   define bitmask_set(N, M)                ((N) | (M))
#   define bitmask_clear(N, M)              ((N) & (~(M)))
#   define bitmask_get_chunk(m, off, size)  __bm_get_chunk((m),(off),(sz))
#endif

u64 __bm_get_chunk(u64 mask, u8 offset, u8 size) {
    int select_mask = 0;
    for(int i = 0; i < size; i++) select_mask |= (1 << i);
    return (mask >> offset) & select_mask;
}

//
// temporary allocator
//
#ifndef TEMP_ALLOCATOR_SIZE // 4 megs
#   define TEMP_ALLOCATOR_SIZE 1024 * 1000 * 4
#endif

static unsigned int  __temp_allocator_current__ ;
static unsigned char __temp_allocator_buffer__  [TEMP_ALLOCATOR_SIZE];

void* temp_alloc(size_t size);
void* temp_put_sized(void* item, size_t size);
void* temp_string(const char* string);


void* temp_alloc(size_t size) {
    assert(size < TEMP_ALLOCATOR_SIZE);
    // reset if can't fit
    if (__temp_allocator_current__ + size > TEMP_ALLOCATOR_SIZE) 
        __temp_allocator_current__ = 0;
    void* mem = __temp_allocator_buffer__ +
                __temp_allocator_current__;
    __temp_allocator_current__ += size;
    return mem;
}

void* temp_put_sized(void* item, size_t size) {
    void* mem = temp_alloc(size);
    memcpy(mem, item, size);
    return mem;
}

void* temp_string(const char* string) {
    return temp_put_sized((void*)string, strlen(string));
}

//
// profiler
//

/* Just an array into which you log your profiling
 * results, index into array is your enum, time saved in nanoseconds.
 */

#ifndef PROFILER_ENTRY_CAPACITY // i believe 512 timer entries is more than enough
#   define PROFILER_ENTRY_CAPACITY 512
#endif

#define MICROSECOND 1000 
#define MILLISECOND 1000*1000 
#define SECOND      1000*1000*1000 

typedef struct {
    bool  finished;
    utime result;
    utime begin;
} __profiler_table_entry__;
static __profiler_table_entry__ 
    __PROFILER_TABLE__ 
        [PROFILER_ENTRY_CAPACITY];

void profiler_begin     (unsigned int entry_id);
void profiler_end       (unsigned int entry_id);
utime profiler_get_ns   (unsigned int entry_id);
utime profiler_get_ms   (unsigned int entry_id);
double profiler_get_sec (unsigned int entry_id);

void profiler_begin(unsigned int entry_id) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    assert(entry_id < PROFILER_ENTRY_CAPACITY);
    __PROFILER_TABLE__[entry_id].finished = false;
    __PROFILER_TABLE__[entry_id].begin =  
        (utime)ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

void profiler_end(unsigned int entry_id) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    assert(entry_id < PROFILER_ENTRY_CAPACITY);
    utime  after = (utime)
        ts.tv_sec * 1000000000LL + ts.tv_nsec;
    utime before = __PROFILER_TABLE__[entry_id].begin;
    utime time_ns = after-before;
    __PROFILER_TABLE__[entry_id].finished = true;
    __PROFILER_TABLE__[entry_id].result = time_ns;
}


double profiler_get_sec(unsigned int entry_id) {
    double time = (double)profiler_get_ns(entry_id)/((double)SECOND);
    return time;
}

utime profiler_get_ms(unsigned int entry_id) {
    utime time = profiler_get_ns(entry_id)/(MILLISECOND);
    return time;
}

utime profiler_get_ns(unsigned int entry_id) {
    assert(__PROFILER_TABLE__[entry_id].finished && "attempt to access a unfinished timer!");
    utime time = __PROFILER_TABLE__[entry_id].result;
    return time;
}


// custom assert

#ifndef FAULT_TRIGGER
#   ifdef  HCH_ASSERT_NO_BREAKPOINT
#       define FAULT_TRIGGER // does nothing 
#   else
#       define FAULT_TRIGGER __asm__("int3")
#   endif
#endif

#ifndef hch_assert
#define hch_assert(COND,...) \
    do { if (!(COND)) { \
        fprintf(stderr,"Assertion at [%s:%s:%d]: ",__FILE__,__func__,__LINE__); \
        fprintf(stderr,__VA_ARGS__); \
        fprintf(stderr,"\n"); \
        FAULT_TRIGGER;      \
        exit(1);            \
    }} while(0)
#endif

#endif // __HCH_PRELUDE_H
