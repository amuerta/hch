/*
    ARGS - argument parsing
*/


/*
    TODO: add flags that don't need '--'
*/
#ifndef __HCH_ARGS_H
#define __HCH_ARGS_H

#include <stdlib.h>  
#include <stdio.h>   
#include <string.h>  
#include <assert.h>  
#include <stdbool.h> 

static char args_error[256];
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

/* TODO: make this function accept multiple args */
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

int arg_flag_with_value(ArgsSlice args, const char* flag, const char** out) {
    int i;
    assert(flag);

    for(i = 0; i < args.count; i++) {
        const char* s = args.items[i];
        if (( s = arg_str_is_flag(s))) {
            const char* save_point = s;
            while(*s && (*s) != '=') s++;
            size_t diff = s - save_point;
            if (*s && !strncmp(save_point, flag, diff)) {
                *out = (++s);
                return i;
            }
        }
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

int arg_single_value(ArgsSlice args, const char* flag, const char** out) {
    ArgsSlice values = {0};
    int location = 0;
    assert(out);
    if((location = arg_list(args, flag, &values))) {
        if(values.count > 1) { 
            memset(args_error, 0, sizeof(args_error));
            strncpy(args_error, "Providing list to a single flag(int)", sizeof(args_error)-1);
            return false;
        } 
        else if(values.count == 1) 
            return *out = *values.items, location;
    }

    const char* value = 0;
    location = arg_flag_with_value(args, flag, &value);
    if(!value)          return false;
    if(!strlen(value))  return false;
    
    *out = value;
    return location;
}

int arg_int(ArgsSlice args, const char* flag,   int* v) {
    const char* value = 0; int n = 0;
    if((n = arg_single_value(args, flag, &value))) *v = atoi(value); 
    return n;
}

int arg_float(ArgsSlice args, const char* flag, float* v) {
    const char* value = 0; int n = 0;
    if((n = arg_single_value(args, flag, &value))) *v = atof(value); 
    return n;
}

int arg_long(ArgsSlice args, const char* flag,  long* v) {
    const char* value = 0; int n = 0;
    if((n = arg_single_value(args, flag, &value))) *v = atol(value); 
    return n;
}

int arg_bool(ArgsSlice args, const char* flag,  bool* v) {
    const char* value = 0; int n = 0;
    if((n = arg_single_value(args, flag, &value))) {
        if(!(!strcmp(value, "true") || !strcmp(value, "false"))) {
            memset(args_error, 0, sizeof(args_error));
            strncpy(args_error, "Providing invalid argument to a single flag(bool)", sizeof(args_error)-1);
            return false;
        }
        *v = !strcmp(value, "true") ? true : false; 
    }
    return n;
}

int arg_string(ArgsSlice args, const char* flag, const char** out) {
    return arg_single_value(args, flag, out);
}

#endif/*__HCH_ARGS_H*/
