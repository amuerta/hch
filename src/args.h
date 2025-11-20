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

bool arg_str_is_flag(const char* str) {
    return str && strlen(str) >= 2 && (strncmp(str,"--", 2)==0 || *str == '-') ;
}

int arg_flag(ArgsSlice args, const char* flag) {
    assert(flag);
    for(int i = 0; i < args.count; i++) {
        const char* s = args.items[i];
        if (arg_str_is_flag(s)) 
            if (*s == '-' || (*s == '-' && strcmp((s+2), flag)==0)) 
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
