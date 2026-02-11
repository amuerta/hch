/*
 *  MEMORY ALLOCATION / MANIPULATION HEADER
 *
 * Example:
 *  ```c
        #include "memory.h"

        #define format      hc_format
        #define temp_alloc  hc_temp_alloc

        int* get_array_of_xs(int n) {
            int *xs = 0;
            xs = temp_alloc(sizeof(int) * n);
            for(int i =0; i<n; i++)
                xs[i] = i;
            return xs;
        }

        int main(void) {
            for(int i = 0; i < 10000; i++) 
                printf("%s\n", format("string %5i", i));
            
            int n = 25;
            int* array = get_array_of_xs(n);
            printf("%s[%i] : [ ", "array" , n);
            for(int i = 0; i < n; i++) {
                printf("%i ", array[i]);
            }
            printf("]\n");
            return 0;
        }
 *  ```
 */

#ifndef __HC_MEMORY_H
#define __HC_MEMORY_H

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h> 

#define hc_zeroed(v)           memset((v), 0, sizeof(*(v)))
#define hc_roptr(v)            ((const void*) v)
#define hc_cmp(l,r)            (memcmp(&(l),&(r),hc_min(sizeof(l),sizeof(r)))==0)

#ifdef HCH_STRIP_MACRO_PREFIX
# define zeroed(v)       hc_zeroed(v)      
# define roptr(v)        hc_roptr(v)       
# define cmp(l,r)        hc_cmp(l,r)       
#endif

#ifndef TEMP_ALLOCATOR_SIZE // 8 Mb
#   define TEMP_ALLOCATOR_SIZE 1024 * 1024 * 8
#endif

/* Format */
const char* hc_format(const char* fmt, ...);

/* Utility */
void* heap(size_t size);
void* recalloc(void* ptr, size_t prev_size, size_t size);

/* Temporary allocator */
#define hc_temp_put(I)\
    hc_temp_put_sized(I, sizeof(*(I)))
void* hc_temp_alloc     (size_t size);
void  hc_temp_reset     (void);
void* hc_temp_put_sized (void* item, size_t size);
void* hc_temp_str       (const char* string);

/*
    # FORMAT
*/
// 16 kb of format.
#define FORMAT_MAX_BUFFERS  16
#define FORMAT_MAX_CHARS    1024
/* C99 or newer */
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
    const char* hc_format(const char* fmt, ...) {
        va_list     args, args_len;
        static int  n;
        static char memory[FORMAT_MAX_BUFFERS][FORMAT_MAX_CHARS];
        int current = n;
        size_t size = 0;
        
        va_start(args, fmt);
        va_copy(args_len, args);
        size = vsnprintf(NULL,0,fmt,args_len);
        va_end(args_len);
        size++; /* vsnprintf is weird. */

        assert(size <= FORMAT_MAX_CHARS);
        vsnprintf((void*)(memory[n]), size, fmt, args);
        memory[n][size-1] = 0; /* vsnprintf is weird. */
        n = (n + 1) % FORMAT_MAX_BUFFERS;

        return memory[current];
    }
#else /* pre-C99 */
    const char* hc_format(const char* fmt, ...) {
        assert(0 && "const char* format(const char* fmt, ...) requires C99+");
    }
#endif

/*
    UTILITY AND ALIASES 
*/


void* heap(size_t size) {
    void* ptr = 0; 
    ptr = malloc(size);
    assert(ptr && "malloc failed in heap(size_t)");
    memset(ptr, 0, size);
    return ptr;
}

void* recalloc(void* ptr, size_t prev_size, size_t size) {
    void* new_ptr = calloc(size, 1);
    if(ptr) {
        memcpy(new_ptr, ptr, prev_size);
        free(ptr);
    }
    return new_ptr;
}

/*
   TEMPORARY ALLOCATOR
*/

void* hc_temp_alloc(size_t size) {
    typedef unsigned int    memindex;
    typedef char            byte;
    
    static memindex current;
    static byte     temp_memory[TEMP_ALLOCATOR_SIZE];
    void*           mem = 0;

    if(size == ((size_t)-1)) {
        current = 0;
        return NULL;
    }

    assert(size < TEMP_ALLOCATOR_SIZE);
    // reset if can't fit
    if (current + size > TEMP_ALLOCATOR_SIZE) 
        current = 0;
    mem = temp_memory + current;
    current += size;
    return mem;
}

void hc_temp_reset(void) {
    const size_t reset_flag = ((size_t)-1);
    (void) hc_temp_alloc(reset_flag);
}

void* hc_temp_put_sized(void* item, size_t size) {
    void* mem = hc_temp_alloc(size);
    memcpy(mem, item, size);
    return mem;
}

void* hc_temp_str(const char* string) {
    return hc_temp_put_sized((void*)string, strlen(string) + 1);
}

#endif /*__HC_MEMORY_H*/
