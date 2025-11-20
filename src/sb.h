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
