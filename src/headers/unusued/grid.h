#ifndef __GRID_H
#define __GRID_H

#include <assert.h>
#include <stdbool.h>

/*FOR default allocator and bump(buffer) allocator.*/
#ifdef HC_INCLUDE_PATH_MEMORY
#   include ""HC_INCLUDE_PATH_MEMORY""
#else
#   include "memory.h"
#endif

/*FOR vector types.*/
#ifdef HC_INCLUDE_PATH_VECTOR_MATH
#   include ""HC_INCLUDE_PATH_VECTOR_MATH""
#else
#   include "vector_math.h"
#endif

#ifndef __ALLOCATOR_INTERFACE
#   error "This library need's allocator interface implementation."
#endif

#define Grid(T) struct {\
    RectangleI bounds;  \
    int cell_size;      \
    T *items;           \
} 
typedef struct {
    RectangleI  bounds;
    unsigned    cell_size;
    void        *items;
} __Grid;


typedef char byte;
typedef Grid(int) IntGrid;

typedef struct GridQueryList {
    struct GridQueryList    *next;
    VectorI2                cell;
} GridQueryList;

typedef struct {
    VectorI2 *items;
    unsigned count, capacity;
} GridQueryBuffer;

#define grid_init(allocator, grid, grid_bounds, cell_size)\
    grid_init_generic(                        \
            (void*)grid, sizeof((grid)->items[0]),      \
            grid_bounds, cell_size,                     \
            allocator)

bool grid_init_generic(
        void* grid, unsigned typesize, 
        RectangleI bounds, unsigned cell_size, 
        Allocator);

bool grid_init_generic(
        void* g, unsigned typesize, 
        RectangleI bounds, unsigned cell_size, 
        Allocator allocator) 
{
    void* buffer = 0;
    unsigned grid_cell_count = 
        bounds.width/cell_size  * 
        bounds.height/cell_size ;
    unsigned grid_size_in_bytes = grid_cell_count * typesize;
    assert(allocator.context_pointer && allocator.alloc &&
            "Expected to have valid allocator");
    buffer = allocator.alloc(
            allocator.context_pointer, 
            grid_size_in_bytes);
    if(!buffer) return false;

    __Grid* grid = g;
    grid->items = buffer;
    grid->bounds = bounds;
    grid->cell_size = cell_size;
    return true;
}

#define ri2rf rectanglei2rectanglef

#define grid_query_fixed_in_rec(grid, result, rec_range)\
    grid_query_fixed_in_rec_generic(\
            (void*) &(grid), sizeof(grid.items[0]),\
            result, rec_range)
// generic 
bool grid_query_fixed_in_rec_generic(
        void* g, unsigned typesize, 
        GridQueryBuffer* out,
        RectangleI range)
{
    const __Grid *grid = (const __Grid*) g;
    RectangleI bounds = grid->bounds;
    int cell_size = grid->cell_size;
    

    VectorI2 
        cells = {
            bounds.width/cell_size,
            bounds.height/cell_size,
        }, origin = {
                  range.x - bounds.x   , 
                  range.y - bounds.y   
        };
    if( origin.x < -range.width ||
        origin.y < -range.height) 
        return false;
    if( origin.x >  bounds.width || 
        origin.y >  bounds.height ) 
        return false;

    unsigned written = 0;
    for(int x = 0; x < cells.x; x++)
        for(int y = 0; y < cells.y; y++) {
            RectangleI cell = {
                .x = bounds.x + x * cell_size,
                .y = bounds.y + y * cell_size,
                .width  = cell_size,
                .height = cell_size,
            }; 
            if(CheckCollisionRecs(ri2rf(cell), ri2rf(range))) {
                DrawRectangleLinesEx(ri2rf(cell), 3, RED);
                if(out->items && out->count < out->capacity) 
                    out->items[out->count++] = vectori2(x,y);
            }
        }
    return true;
}

#define grid_draw(grid) \
    grid_draw_generic((void*) &(grid), sizeof(grid.items[0]))
void grid_draw_generic(void* g, unsigned typesize) {
    const __Grid *grid = (const __Grid*) g;
    RectangleI bounds = grid->bounds;
    int cell_size = grid->cell_size;

    VectorI2 cells = {
        bounds.width/cell_size,
        bounds.height/cell_size,
    };


    for(int x = 0; x < cells.x; x++)
        for(int y = 0; y < cells.y; y++) {
            Rectangle cell = {
                .x = bounds.x + x * cell_size,
                .y = bounds.y + y * cell_size,
                .width  = cell_size,
                .height = cell_size,
            }; 
            DrawRectangleLinesEx(cell, 1, GRAY);
        }

    DrawRectangleLinesEx(ri2rf(bounds),1,GRAY);
}

#define grid_get(grid, x, y)\
    (grid.items[grid_index_assert((void*) &(grid), x, y)])
unsigned grid_index_assert(void* g, unsigned x, unsigned y) {
    const __Grid *grid = (const __Grid*) g;
    RectangleI bounds = grid->bounds;
    unsigned cell_size = grid->cell_size;
    assert(cell_size > 0 && bounds.width > 0 && bounds.height > 0);
    VectorI2 grid_size = {
        bounds.width/cell_size ,
        bounds.height/cell_size 
    };
    unsigned grid_cell_count = grid_size.x * grid_size.y;
    unsigned index = x + (y * grid_size.x);
    assert(index < grid_cell_count && "Grid overflow.");
    return index;
}
#endif/*__GRID_H*/
