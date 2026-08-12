#ifndef __HC_STACK_H
#define __HC_STACK_H

/*
 *  STACK - a small snippet build on top of the Dynamic Array.
 *  This IS just a set of macros for dynamic array or buffer.
 */

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define hc_Stack(T) struct/*Stack(T)*/{    \
    T* items;                   \
    size_t count, capacity;     \
}

#define stk_push(stack, item) do {                                  \
    assert((stack)->count < (stack)->capacity && "Stack overflow"); \
    (stack)->items[(stack)->count++] = item;                        \
} while(0)

#define stk_pop(stack) \
    ((stack)->items[stk_pop_helper(&((stack)->count),  (stack)->capacity)])

size_t stk_pop_helper(size_t* count, size_t capacity) {
    assert(count && "Invalid pointer, weird bug??");
    assert( (*count) > 0 && "Stack underflow");  
    size_t index = (*count) - 1;
    (*count)--;
    return index;
}

#endif /*__HC_STACK_H*/
