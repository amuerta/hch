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

/*You can increase entries count if you desire so.*/
#ifndef ARGS_INFO_MAX_ENTRIES
#   define ARGS_INFO_MAX_ENTRIES 64
#endif

#ifndef ARGS_INFO_STRING_BUFFER_SIZE
#   define ARGS_INFO_STRING_BUFFER_SIZE (1<<10)
#endif

#ifndef ARGS_ERROR_BUFFER_SIZE
#   define ARGS_ERROR_BUFFER_SIZE (1<<9)
#endif

/*Using this you can collect all the checked flags
 * into neatly formated "USAGE" table that then
 * can be printed at the end after all the flags 
 * where checked.*/

typedef struct {
    unsigned count, relative_pointer;    
} ArgRecordSlice;

typedef struct {
    ArgRecordSlice flag;
    ArgRecordSlice type;
    ArgRecordSlice description;
} ArgRecord;

#define arg_min(a,b) (((a) < (b)) ? (a) : (b))
#define arg_max(a,b) (((a) > (b)) ? (a) : (b))

typedef struct {
    unsigned        flag_padding;
    unsigned        usage_padding;
    unsigned        biggest_flag_length;
    unsigned        biggest_type_length;
    const char*     executable_path;

    struct {
        unsigned    count;
        ArgRecord   items   [ARGS_INFO_MAX_ENTRIES];
    } flags;

    struct {
        unsigned    count;
        char        buffer  [ARGS_INFO_STRING_BUFFER_SIZE];
    } storage;
} ArgsRecord;

typedef struct {
    char**          items;
    int             count;
    ArgsRecord*     record;
    bool            falltrough;
    char            errors[ARGS_ERROR_BUFFER_SIZE];
} Args;

#define arg_flag(args, flag)\
    arg_flag_record(args, flag, "")

#define arg_int(args, v, flag) \
    arg_int_record(args, v,     flag,  "")

#define arg_float(args,v, flag) \
    arg_float_record(args, v,   flag,  "")

#define arg_long(args,v, flag) \
    arg_long_record(args, v,    flag,  "")

#define arg_bool(args,v, flag) \
    arg_bool_record(args, v,    flag,  "")

#define arg_string(args,v, flag) \
    arg_string_record(args, v,  flag,  "")

#define arg_list(args, out, flag)\
    arg_list_record(args, out, flag, "Any", "", out)


void* argsrecord_alloc(ArgsRecord* record, size_t size) {
    void* ptr = record->storage.buffer + record->storage.count;
    if(record->storage.count + size > sizeof(record->storage.buffer))
        return NULL;
    record->storage.count += size;
    return ptr;
}

ArgRecordSlice argsrecord_put_slice(ArgsRecord* record, const char* string) {
    unsigned length = 0;
    char* storage_pointer = NULL;
    unsigned relative_pointer = 0;
    ArgRecordSlice slice = {0};
    if(!string)         return slice;
    if(!strlen(string)) return slice;

    length = strlen(string);
    storage_pointer = argsrecord_alloc(record, length);
    relative_pointer = storage_pointer - record->storage.buffer;
    memcpy(storage_pointer, string, length);

    slice.count = length;
    slice.relative_pointer = relative_pointer;
     return slice;
}

bool argsrecord_append_flag(ArgsRecord *record, 
        const char* flag, 
        const char* type,
        const char* description,
        const char* additions) 
{
    if(!record) return false;
    unsigned i = 0;
    static char work_buffer[1<<8];

    /* verify if flags was mentioned. */
    for(i = 0; i < record->flags.count; i++) {
        ArgRecord item = record->flags.items[i];
        void* string_in_buffer = 
            record->storage.buffer + item.flag.relative_pointer;
        if(!strncmp(flag, string_in_buffer, item.flag.count)) 
            return false; /* already exists. */ 
    }

    ArgRecord new_flag_record = {0};
    record->biggest_flag_length = arg_max(record->biggest_flag_length, strlen(flag));
    record->biggest_type_length = arg_max(record->biggest_type_length, strlen(type));
    /*place into the memory a flag*/


    new_flag_record.flag = argsrecord_put_slice(record, flag);
    new_flag_record.type = argsrecord_put_slice(record, type);

    snprintf(work_buffer, sizeof(work_buffer)-1, 
        "(%s) : %s. %s",
        type, description, additions
    );

    new_flag_record.description = argsrecord_put_slice(record, description);

    /*append it to record*/
    record->flags.items[record->flags.count++] = new_flag_record;

    return true;
}

void argsrecord_print(ArgsRecord record) {
    unsigned i = 0;
    const char* buffer = record.storage.buffer;
    printf("%*sUSAGE: %s [<FLAGS>]\n"
            "%*s > WHERE <FLAGS> DEFINED AS:\n"

            , 
            record.usage_padding, "",
            record.executable_path,
            record.usage_padding, ""
          );
    for(i = 0; i < record.flags.count; i++) {
        ArgRecord item = record.flags.items[i];
        const char* flag        = buffer + item.flag.relative_pointer;
        const char* type        = buffer + item.type.relative_pointer;
        const char* description = buffer + item.description.relative_pointer;
        printf(
                "%*s"
                " "
                "\"--%-*.*s\""
                " :"
                "%-*.*s" 
                " -- "
                "%.*s\n",

                record.flag_padding, " ",
                
                record.biggest_flag_length,
                item.flag.count, flag,

                record.biggest_type_length,
                item.type.count, type,
                
                item.description.count, description
        );
    }
}

const char* arg_str_is_flag(const char* str) {
    const char* flag = str;
    bool valid_flag = flag && strlen(flag) >= 2;
    if (!valid_flag)  return NULL;
    if (*flag == '-') flag++; else return NULL;
    if (*flag == '-') flag++;
    return flag;
}


/* TODO: make `arg_flags` function that accept's multiple arguments 
 * and bundles them under the same description and type. IN SHORT - Aliases.*/
int arg_flag_record(Args args, const char* flag, const char* description) {
    assert(flag);

    if(description)
        argsrecord_append_flag(args.record, flag, "none", description, "");
    for(int i = 0; i < args.count; i++) {
        const char* s = args.items[i];
        if (( s = arg_str_is_flag(s))) 
            if (strcmp(s, flag)==0 && !args.falltrough) {
                return i;
            }
    }
    return 0;
}

int arg_flag_with_value(Args args, 
        const char* flag, 
        const char* type, 
        const char* description, 
        const char** out) 
{

    argsrecord_append_flag(args.record, flag, type, description, "");
    int i = 0;
    assert(flag);
    for(i = 0; i < args.count; i++) {
        const char* s = args.items[i];
        if (( s = arg_str_is_flag(s))) {
            const char* save_point = s;
            while(*s && (*s) != '=') s++;
            size_t diff = s - save_point;
            if (*s && !strncmp(save_point, flag, diff) && !args.falltrough) {
                *out = (++s);
                return i;
            }
        }
    }
    return 0;
}

int arg_list_record(Args args, Args* out,
        const char* flag, const char* type, const char* description) 
{
    char modify_type_buffer[128] = {0};
    snprintf(modify_type_buffer, sizeof(modify_type_buffer)-1, 
            "%s[]", 
            type);
    if(flag && type && description)
        argsrecord_append_flag(args.record, flag, modify_type_buffer, description, "");
    int b = 0;
    if((b = arg_flag_record(args, flag, NULL))) {
        if (b+1 < args.count)   out->items = args.items + b+1;
        else                    out->items = 0;
        for(int i = b+1; i < args.count; i++) {
            if(arg_str_is_flag(args.items[i])) break;
            out->count++;
        }
    }
    return b;
}

int arg_single_value(Args *args, 
        const char* flag, 
        const char* type, 
        const char* description, 
        const char** out) {
    Args values = {0};
    int location = 0;
    assert(out);
    if((location = arg_list_record(*args, &values, flag, NULL, NULL))) {
        if(values.count > 1) {
            char* new_error = args->errors + strlen(args->errors);
            unsigned available_size =  ((sizeof(args->errors)-1) - strlen(args->errors));
            snprintf(new_error, available_size,
                    " - Providing list to a single flag `%s` (int,float,long)\n", 
                    flag
            );
            return false;
        } 
        else if(values.count == 1) 
            return *out = *values.items, location;
    }

    const char* value = 0;
    location = arg_flag_with_value(*args, flag, type, description, &value);
    if(!value)          return false;
    if(!strlen(value))  return false;
    
    *out = value;
    return location;
}

int arg_int_record(Args *args, int* v,
        const char* flag, const char* description)
{
    const char* value = 0; int n = 0;
    if((n = arg_single_value(args, flag, "int", description, &value))) *v = atoi(value); 
    return n;
}

int arg_float_record(Args *args, float* v,
        const char* flag, const char* description) {
    const char* value = 0; int n = 0;
    if((n = arg_single_value(args, flag, "float", description, &value))) *v = atof(value); 
    return n;
}

int arg_long_record(Args *args, long* v,
        const char* flag,  const char* description) {
    const char* value = 0; int n = 0;
    if((n = arg_single_value(args, flag, "long", description,&value))) *v = atol(value); 
    return n;
}

int arg_bool_record(Args *args, bool* v,
        const char* flag, const char* description) {
    const char* value = 0; int n = 0;
    if((n = arg_single_value(args, flag, "bool", description,&value))) {
        if(!(!strcmp(value, "true") || !strcmp(value, "false"))) {
            char* new_error = args->errors + strlen(args->errors);
            unsigned available_size =  ((sizeof(args->errors)-1) - strlen(args->errors));
            snprintf(new_error, available_size,
                    " - Providing incorrect value to a single-value flag `%s` (bool)\n", 
                    flag
            );
            return false;
        }
        *v = !strcmp(value, "true") ? true : false; 
    }
    return n;
}

int arg_string_record(Args* args, const char** out,
        const char* flag, 
        const char* description) 
{
    return arg_single_value(args, flag, "string", description, out);
}

#endif/*__HCH_ARGS_H*/
