#include "../src/memory.h"

#define format      hc_format
#define temp_alloc    hc_temp_alloc

int* get_array_of_xs(int n) {
    int *xs = 0;
    xs = temp_alloc(sizeof(int) * n);
    for(int i =0; i<n; i++)
        xs[i] = i;
    return xs;
}

typedef unsigned SystemMemoryFlags; enum {
    HC_MEMORY_DEFAULT   = 0, // READ + WRITE
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

// MMAP likes to work with pages less or equal to this.
// https://stackoverflow.com/questions/28826470/mmap-failed-when-trying-to-map-huge-page-1gb
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
#ifdef _WIN32

#else // POSIX: LINUX, BSD
    // prot
    system_allocator_protection |= (flags & HC_MEMORY_READ)  ? PROT_READ :0;
    system_allocator_protection |= (flags & HC_MEMORY_WRITE) ? PROT_WRITE:0;
    system_allocator_protection |= (flags & HC_MEMORY_EXEC)  ? PROT_EXEC :0;
    // flags
    system_allocator_flags |= (flags & HC_MEMORY_SHARED)? MAP_SHARED    : MAP_PRIVATE;
    system_allocator_flags |= !(flags & HC_MEMORY_FILE) ? MAP_ANONYMOUS : 0;

    if (!flags) {
        system_allocator_protection = PROT_READ | PROT_WRITE;
        system_allocator_flags      = MAP_PRIVATE | MAP_ANONYMOUS;
    }
    if (!(flags & HC_MEMORY_SUPPORT_HUGE_PAGES))
        assert("Regular pages are up to 1GB in size." && size <= HC_MEMORY_MAX_PAGE_SIZE);
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
#ifdef _WIN32
#else // POSIX
    bool result = !munmap(page->ptr, (assert(page->size), page->size));
    memset(page, 0, sizeof(*page));
    return result;
#endif
}

int main(void) {
    // for(int i = 0; i < 10000; i++) 
        // printf("%s\n", format("string %5i", i));
    
    // size_t n = (1<<30);
    size_t n = (1<<30);
    // SystemMemoryPage page = hc_memory_map(n, HC_MEMORY_DEFAULT); 
    page.ptr = malloc(n);
    page.size = n;
    assert(page.ptr);
    int* array = page.ptr;
    //get_array_of_xs(n);
    scanf("%lu", &n);
#if 0
    printf("%s[%i] : [ ", "array" , n);
    for(int i = 0; i < n; i++) {
        printf("%i ", array[i]);
    }
    printf("]\n");
#endif
    free(page.ptr);
    // assert(hc_memory_unmap(&page));
    return 0;
}
