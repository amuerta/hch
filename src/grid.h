#ifndef __HC_GRID_H
#define __HC_GRID_H

#include <assert.h>
#include <string.h>
#include <stdlib.h>

#ifndef HC_GRID_DISABLE_BOUNDS_CHECKING
#   define hc_grid_optional_bounds_checking hc_grid_bound_check
#else
#   define hc_grid_optional_bounds_checking(g,x,y) 0
#endif

#define hc_Grid(T) struct {\
    T* items;\
    size_t width, height, items_size_in_bytes;\
}

#define hc_grid_at(grid, x, y) \
    grid.items[(x+(grid.width)*y)\
    + hc_grid_optional_bounds_checking(&grid, (size_t)x,(size_t)y)]

#define hc_grid_from_heap(grid, w, h)\
    hc_grid_from_heap_ex(grid, sizeof((grid)->items[0]), w, h)

#define hc_grid_from_buffer(grid, memory, size, width) \
    hc_grid_from_buffer_ex(\
            grid, sizeof((grid)->items[0]),\
            memory, size, \
            width) 

typedef struct {
    void*   items;
    size_t  width, height, items_size_in_bytes;
} hc_GridBase;

static inline size_t 
        hc_grid_measure(size_t t, size_t w, size_t h);
size_t  hc_grid_bound_check(void* g, size_t x, size_t y);
void    hc_grid_reset(void* g);
void    hc_grid_from_buffer_ex(void* g, size_t typesize, void* memory, size_t size, size_t width);
void    hc_grid_from_heap_ex(void* g, size_t typesize, size_t w, size_t h);
void    hc_grid_free_heap(void* g);

static inline size_t 
hc_grid_measure(size_t t, size_t w, size_t h) {
    return t*w*h; 
}

size_t hc_grid_bound_check(void* g, size_t x, size_t y) {
    hc_GridBase* grid = g;
    assert(x < grid->width && y < grid->width && "Out of bounds of grid");
    return 0;
}

void hc_grid_reset(void* g) {
    hc_GridBase* grid = g;
    memset(grid->items, 0, grid->items_size_in_bytes);
}

void hc_grid_from_buffer_ex(
        void* g, size_t typesize, 
        void* memory, size_t size, 
        size_t width) 
{
    hc_GridBase* grid = g;
    size_t w, h; 
    h = (size/typesize) / width;
    assert(h > 0 && "buffer too small to be able to fit at least a row of elements");
    w = width;

    grid->items = memory;
    grid->items_size_in_bytes = size;
    grid->width = w;
    grid->height = h;
}

void hc_grid_from_heap_ex(void* g, size_t typesize, size_t w, size_t h) {
    size_t  size   = hc_grid_measure(typesize, w, h);
    void*   memory = calloc(size, 1);
    hc_grid_from_buffer_ex(g, typesize, memory, size, w);
}

void hc_grid_free_heap(void* g) {
    hc_GridBase* grid = g;
    free(grid->items);
    memset(g,0,sizeof(*grid));
}

#endif /*__HC_GRID_H*/
