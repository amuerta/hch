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
#include <stdbool.h> 

#define hc_zeroed(v)            memset((v), 0, sizeof(*(v)))
#define hc_roptr(v)             ((const void*) v)
#define hc_cmp(l,r)             ((sizeof(l)==sizeof(r)) && (memcmp(&(l),&(r),sizeof(l))==0))
#define hc_partcmp(l,r)         (memcmp(&(l),&(r),hc_min(sizeof(l),sizeof(r)))==0)
#define hc_alignof(size, alignment)\
                                (((size) + (alignment) - 1) & ~((alignment) - 1))

#ifdef HCH_STRIP_MACRO_PREFIX
# define zeroed(v)       hc_zeroed(v)      
# define roptr(v)        hc_roptr(v)       
# define cmp(l,r)        hc_cmp(l,r)       
#endif

/* 16 Mb should be plenty for all uses. */
#ifndef TEMP_ALLOCATOR_SIZE /* 16 Mb */
#   define TEMP_ALLOCATOR_SIZE 1024 * 1024 * 16
#endif

/*
    # ALLOCATOR INTERFACE
*/
/*I just want to have a generic "This is allocator" type for
 * EVERY DATA STRUCTURE that I have ever made to avoid repeating 
 * myself hundreds of times with "type_from_buffer" or
 * "type_from_static_buffer_sized" or other nonsense.
*/
#include <stddef.h>
enum {
    /*You can decide if allocator callback should stay 
     * silent it its NULL, or if it's a bug that should
     * be asserted immidately.*/
    ALLOCATOR_FLAG_ASSERT_CONTEXT   = 1<<0,
    ALLOCATOR_FLAG_ASSERT_ALLOC     = 1<<1,
    ALLOCATOR_FLAG_ASSERT_FREE      = 1<<2,
    ALLOCATOR_FLAG_ASSERT_REALLOC   = 1<<3,
} AllocatorFlags;

/*Allocator can be have up to 256 bytes of context*/
/*  + Customizable  */
#ifndef ALLOCATOR_MAX_OWNED_SIZE
#   define ALLOCATOR_MAX_OWNED_SIZE ((1<<8)/4) /*bytes are stored in unsigned[].*/
#endif


#define __ALLOCATOR_INTERFACE
/*TODO: Allocator interface ownership of allocator's context. */
/*TODO: Allocator "contract" flag dictates what operations user should expect from an allocator.*/
typedef struct {
    /*size_t is here to just preserve alignement*/   
    size_t  flags; 
    void*   context_pointer;
    void*   (*alloc)(void* ctx, size_t size);
    void    (*free) (void* ctx, void* ptr, size_t size); 
    /*                                     ^^^^^^^^^^^*/
    /*size is optional, and is only needed for system allocators
     * that _sometimes_ need size of allocated block that you are freeing.*/
    
    void*   (*realloc) (void* ctx, void* ptr, size_t old_size, size_t new_size); 
    /*      ^^^^^^^^^^*/
    /*this function would be useless if not for the allocator that
     * can benefit from specific operation of repurposing memory.
     * this will be used only by data structures risizing of which 
     * is very expensive.*/
    
    void    (*reset)  (void* ctx);
    void    (*drop)   (void* ctx);
    /*      ^^^^^^^x2 
     * For allocator such as Arena and Bump.*/
    
    unsigned context_buffer[ALLOCATOR_MAX_OWNED_SIZE];
} Allocator;

/*You can nest allocators within allocator to have exact behaviour
 * you desire for the algorithm or data structure you need memory
 * allocations to be perfomed for.*/

#define allocator_alloc(allocator, size)\
        ((allocator).alloc((allocator).context_pointer, (size))) \

#define allocator_realloc(allocator, ptr, old_size, new_size)\
        ((allocator).realloc((allocator).context_pointer,\
                          (ptr), (old_size), (new_size)) )\

#define allocator_free(allocator, ptr, size)\
        ((allocator).free((allocator).context_pointer, (ptr), (size)) )

/*Allocator support for malloc*/
Allocator allocator_malloc(void);
void*   allocator_malloc_alloc_callback    (void*, size_t        );
void    allocator_malloc_free_callback     (void*, void* , size_t);
void*   allocator_malloc_realloc_callback  (void*, void* , size_t, size_t);

/*Allocator support for temporary allocator*/
Allocator allocator_temp(void);
void*   allocator_temp_alloc_callback      (void*, size_t );

/* Format */
const char* hc_format(const char* fmt, ...);

/* Utility */
void*   heap(size_t size);
void*   recalloc(void* ptr, size_t prev_size, size_t size);
size_t  hch_nearest_pow2(size_t x);
bool    hch_is_pow2(size_t n);
    

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
/* 32 KB of format. */
#define FORMAT_MAX_BUFFERS  16
#define FORMAT_MAX_CHARS    (1024 * 2)
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
    UTILITIES
*/
size_t hch_nearest_pow2(size_t x) {
    size_t p = 1;
    while(p < x) p*=2;
    return p;
}

bool hch_is_pow2(size_t n) {
    return (n & (n-1)) == 0;
}

void* heap(size_t size) {
    void* ptr = 0; 
    ptr = malloc(size);
    assert(ptr && "malloc failed in `void* heap(size_t)`");
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

/*  Allocator interface integration */
Allocator allocator_malloc(void) {
    Allocator a = {
        .context_pointer = NULL,
        .alloc = allocator_malloc_alloc_callback,
        .free = allocator_malloc_free_callback,
        .realloc = allocator_malloc_realloc_callback,
    }; return a;
}
void* allocator_malloc_alloc_callback(void* ctx, size_t size) {
    /*unsused*/ 
    (void) (ctx);
    return memset(malloc(size),0,size);
}
void allocator_malloc_free_callback(void* ctx, void* ptr, size_t block) {
    /*unsused*/ 
    (void) (ctx);
    (void) (block);
    assert(ptr && "Trying to free NULL. Don't.");
    free(ptr);
}
void* allocator_malloc_realloc_callback(void* ctx, void* ptr, size_t old_size, size_t new_size) {
    /*unsused*/ 
    (void) (ctx);
    void* new_ptr = malloc(new_size);
    memcpy(new_ptr, ptr, old_size);
    if(ptr) free(ptr);
    assert(new_ptr && "malloc failed.");
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
    assert(size < TEMP_ALLOCATOR_SIZE &&
            "Allocation is too big for temporary allocation."
    );
    /* reset if can't fit */
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



/*  Allocator interface integration */
void* allocator_temp_alloc_callback(void* ctx, size_t size) {
    /*unsused*/ 
    (void) (ctx);
    return hc_temp_alloc(size);
}

Allocator allocator_temp(void) {
    Allocator a = {
        .flags   =  ALLOCATOR_FLAG_ASSERT_FREE      | 
                    ALLOCATOR_FLAG_ASSERT_REALLOC   ,
        .context_pointer     = NULL,
        .alloc   = allocator_temp_alloc_callback,
        .free    = NULL, /*Doesn't support freeing*/
        .realloc = NULL, /*Doesn't support resizing allocation*/
    }; return a;
}



/*
   CROSS PLATFORM MMAP 
   Support:
    POSIX       - mmap
    WINDOWS     - VirutalAlloc
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
typedef unsigned SystemMemoryFlags; enum {
    HC_MEMORY_DEFAULT   = 0, /*READ + WRITE */ 
    HC_MEMORY_READ      = 0x1,
    HC_MEMORY_WRITE     = 0x2,
    HC_MEMORY_EXEC      = 0x4,
    HC_MEMORY_SHARED    = 0x8,
    HC_MEMORY_FILE      = 0x10,
    HC_MEMORY_SUPPORT_HUGE_PAGES = 0x20,
};


typedef struct {
    void*   ptr;
    size_t  size;
} SystemMemoryPage;

/*
// MMAP likes to work with pages less or equal to this.
// https://stackoverflow.com/questions/28826470/mmap-failed-when-trying-to-map-huge-page-1gb
*/
#define HC_MEMORY_MAX_PAGE_SIZE (1<<30)

#ifdef _WIN32
#   include    <memoryapi.h>
#else
#   include    <sys/mman.h>
#endif


SystemMemoryPage hc_memory_map(size_t size, SystemMemoryFlags flags) {
    SystemMemoryPage page = {0};
    unsigned system_allocator_flags = 0,
             system_allocator_protection = 0;


    bool read    = flags & HC_MEMORY_READ,
         write   = flags & HC_MEMORY_WRITE,
         execute = flags & HC_MEMORY_EXEC;
#ifdef _WIN32

    if(flags & HC_MEMORY_FILE) {
        assert(0 && "TODO: MAP FILES TO MEMORY ON WINDOWS");
    } 
    else system_allocator_flags = MEM_COMMIT;

    if(flags & HC_MEMORY_SUPPORT_HUGE_PAGES) 
        system_allocator_flags |= MEM_LARGE_PAGES;
    else 
        assert("Regular pages are up to 1GB in size." && size <= HC_MEMORY_MAX_PAGE_SIZE);

    if (read && write && execute)   system_allocator_protection = PAGE_EXECUTE_READWRITE;
    else if (read && execute)       system_allocator_protection = PAGE_EXECUTE_READ;
    else if (read && write)         system_allocator_protection = PAGE_READWRITE;
    else if (execute)               system_allocator_protection = PAGE_EXECUTE;
    else if (read)                  system_allocator_protection = PAGE_READONLY;
    else if (!flags) {
        system_allocator_protection = PAGE_READWRITE;
        system_allocator_flags      = MEM_COMMIT;
    }
    else                            system_allocator_protection = PAGE_NOACCESS;

    page.ptr = VirtualAlloc(NULL, size, 
            system_allocator_flags, 
            system_allocator_protection);
    page.size = size;
/* ifdef _WIN32 */


#else /* POSIX: LINUX, BSD */
    /* protection */
    system_allocator_protection |= (read)  ? PROT_READ :0;
    system_allocator_protection |= (write) ? PROT_WRITE:0;
    system_allocator_protection |= (execute)  ? PROT_EXEC :0;
    /* flags */
    system_allocator_flags |= (flags & HC_MEMORY_SHARED)? MAP_SHARED    : MAP_PRIVATE;
    system_allocator_flags |= !(flags & HC_MEMORY_FILE) ? MAP_ANONYMOUS : 0;


    if (flags & HC_MEMORY_SUPPORT_HUGE_PAGES)
        system_allocator_flags |= MAP_HUGETLB;
    else
        assert("Regular pages are up to 1GB in size." && size <= HC_MEMORY_MAX_PAGE_SIZE);
    

    if (!flags) {
        system_allocator_protection = PROT_READ | PROT_WRITE;
        system_allocator_flags      = MAP_PRIVATE | MAP_ANONYMOUS;
    }

    page.ptr = mmap(NULL, size, 
            system_allocator_protection, 
            system_allocator_flags,
            -1,0);
    page.size = size;
    if(page.ptr == MAP_FAILED) memset(&page,0,sizeof(page));
#endif

    return page;
}

bool hc_memory_unmap(SystemMemoryPage *page) {
    bool result = false;

#ifdef _WIN32
    result = VirtualFree(page->ptr, page->size, MEM_DECOMMIT);
    memset(page, 0, sizeof(*page));
    return result;
/* ifdef _WIN32 */

#else /* POSIX */
    result = !munmap(page->ptr, (assert(page->size), page->size));
    memset(page, 0, sizeof(*page));
    return result;
#endif

}

#endif /*__HC_MEMORY_H*/
