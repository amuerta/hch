//
// PRELUDE
//

#ifndef __HCH_PRELUDE_H
#define __HCH_PRELUDE_H


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

#endif // __HCH_PRELUDE_H

//
// MACROS
//

#define arrlen(a)           (sizeof(a)/sizeof(a[0]))
#define cast(v, T)          ((T)v)
#define transmute(v, T)     *((T*)&(v))
#define zeroed(v)           memset(&(v), 0, sizeof(v))
#define unused(v)           ((void) (v))
#define BREAKPOINT()        __asm__("int3")

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

#ifndef range
#   define range(n, min, max)  ((n)>=(min) && (n)<=(max))
#endif

#ifndef clamp
#   define clamp(n, min, max)  \
    ((n) < (min)) ? (min) : ((n) > (max) ? (max) : (n)) 
#endif

//
// bitmasking
//

#ifndef HCH_FULL_BITMASK_PREFIX
#   define bm_toggle(N, M) ((N) ^ (M))
#   define bm_set(N, M)    ((N) | (M))
#   define bm_clear(N, M)  ((N) & (~(M)))
#   define bm_get_chunk(m, off, size) __bm_get_chunk((m),(off),(sz))
#else
#   define bitmask_toggle(N, M) ((N) ^ (M))
#   define bitmask_set(N, M)    ((N) | (M))
#   define bitmask_clear(N, M)  ((N) & (~(M)))
#   define bitmask_get_chunk(m, off, size) __bm_get_chunk((m),(off),(sz))
#endif

u64 __bm_get_chunk(u64 mask, u8 offset, u8 size) {
    int select_mask = 0;
    for(int i = 0; i < size; i++) select_mask |= (1 << i);
    return (mask >> offset) & select_mask;
}

// custom assert

#ifdef  HCH_ASSERT_NO_BREAKPOINT
#define FAULT_TRIGGER // does nothing 
#else
#define FAULT_TRIGGER __asm__("int3")
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
