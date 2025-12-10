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

const char* arg_str_is_flag(const char* str) {
    const char* flag = str;
    bool valid_flag = flag && strlen(flag) >= 2;
    if (!valid_flag)  return NULL;
    if (*flag == '-') flag++; else return NULL;
    if (*flag == '-') flag++;
    return flag;
}

int arg_flag(ArgsSlice args, const char* flag) {
    assert(flag);
    for(int i = 0; i < args.count; i++) {
        const char* s = args.items[i];
        if (( s = arg_str_is_flag(s))) 
            if (strcmp(s, flag)==0) 
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
