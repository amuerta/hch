/*LibC*/
#ifndef __HC_STRING_OR_STRING_BUILDER_H
#define __HC_STRING_OR_STRING_BUILDER_H
#   include <stdlib.h>  
#   include <stdio.h>   
#   include <string.h>  
#   include <assert.h>  
#   include <stdarg.h>
#   include <stdbool.h>
#endif


/****************
 ****************
 * String 
 ****************
 ****************/
#ifndef __HC_STRING_H
#define __HC_STRING_H


#define STR_TEMP_SIZE 1024
#define STR_NOPATTERN -1

#ifndef HC_HINT_INLINE
#   if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#       define HC_HINT_INLINE static inline
#   else 
#       define HC_HINT_INLINE /**/
#   endif
#endif

#define hc_string__max(A,B) (A) > (B) ? (A) : (B)
#define hc_string__min(A,B) (A) > (B) ? (B) : (A)
#define hc_string__clamp(n, min, max) \
     ((n) < (min)) ? (min) : ((n) > (max) ? (max) : (n)) 
#define hc_string_loop(I,N) for(size_t I = 0; I < (N); I++)

// Those are just name alias.
typedef const char* hc_CString;
typedef char*       hc_MutableCString;

typedef struct {
    size_t 	    count;
	const char* items;
} hc_String;

/* TODO:
   Write access is avilable for all functions that return void
   by default items is read only but some functions may modify it 
   via forceful mutable cast */
HC_HINT_INLINE hc_String 	hc_string_make(hc_CString cstr); 
HC_HINT_INLINE hc_String 	hc_string_make_sized(hc_CString str, size_t count); 
HC_HINT_INLINE hc_String 	hc_string_dup(hc_String orig);
HC_HINT_INLINE bool 	    hc_string_is_empty(hc_String s);
HC_HINT_INLINE void 	    hc_string_reset(hc_String* s);

#define         hc_string_print(s) hc_string_print_fmt(s,0,0)
#define         hc_string_format(s) ((int)s.count), (s.items)
hc_String       hc_string_trim(hc_String s);
hc_String       hc_string_trim_left(hc_String s);
hc_String       hc_string_trim_right(hc_String s);
hc_String       hc_string_substr(hc_String orig, size_t index, size_t count);
hc_String       hc_string_split_by_chars(hc_String *s, const char* chars);
bool            hc_string_begins_with(hc_String src, hc_String pat);
bool            hc_string_ends_with(hc_String src, hc_String pat);
bool            hc_string_cmp(hc_String l, hc_String r);
bool            hc_string_cmp_cstr(hc_String l, const char* str);
int             hc_string_seek_pattern(hc_String src, hc_String pat);
bool            hc_string_is_integer(hc_String s);
bool            hc_string_is_float(hc_String s);
void            hc_string_print_fmt(hc_String s, char* prefix, char* postfix);
hc_CString      hc_string_temp_cstr(hc_String s);
	
//
// IMPLEMENTATION
//


//
// CString and MutableCString.
//



void cstring_bumpwrite(hc_MutableCString *str, hc_CString content) {
    strcpy(*str, content);
    *str += strlen(content);
}

//
// String (length + ptr)
//

HC_HINT_INLINE hc_String hc_string_zero(void) {
    hc_String s = {0};
    return s;
}

HC_HINT_INLINE hc_String hc_string_make_sized(const char* cstr, size_t size) {
    hc_String s = {
        .count = size,
        .items = cstr,
    };
    return s;
}

HC_HINT_INLINE hc_String hc_string_make(const char* cstr) {
    return hc_string_make_sized(cstr, strlen(cstr));
}

HC_HINT_INLINE hc_String hc_string_dup(hc_String s) {
	hc_String dup = {0};
    memcpy(&dup, &s, sizeof(s));
	return dup;
}


hc_String hc_string_range(hc_String orig, size_t begin, size_t end) {
    assert(begin == end && "can't have slice of size 0");
    int b = hc_string__min(begin, end);
    int e = hc_string__max(begin, end);
    b = hc_string__clamp(b, 0, (int)orig.count - 1);
    e = hc_string__clamp(e, 0, (int)orig.count - 1);

    hc_String s = {
        .items = orig.items + begin,
        .count = end - begin,
    };

    return s;
}

hc_String hc_string_substr(hc_String orig, size_t index, size_t count) {
	assert(index < orig.count && 
			"Index overflows original string");
	assert(count < orig.count && 
			"Sub string countgth cannot be greated than origin count");
	assert(index + count <= orig.count && 
			"Slice go out of original string bounds");


    hc_String sub = {
        .items = orig.items + index,
        .count = count,
    };
	return sub;
}



bool hc_string_begins_with(hc_String src, hc_String pat) {
	assert(src.count != 0 && pat.count != 0 && 
			"Source and pattern do not allow countgth of 0");
	assert(src.count > pat.count && 
			"Pattern string cannot be "
			"equal or bigger to Source!"
			"Check hc_string_equal(hc_String,hc_String)"
			);

	bool equal = true;
	for (uint i = 0; i < pat.count; i++)
		equal = equal && (src.items[i] == pat.items[i]);
	return equal;
}

bool hc_string_ends_with(hc_String src, hc_String pat) {
	assert(src.count != 0 && pat.count != 0 && 
			"Source and pattern do not allow countgth of 0");
	assert(src.count > pat.count && 
			"Pattern string cannot be "
			"equal or bigger to Source!"
			"Check hc_string_equal(hc_String,hc_String)"
			);

	bool equal = true;
	size_t offset = src.count - pat.count;
	for (uint i = (pat.count - 1); i > 0; i--) {
		equal = equal && (src.items[i+offset] == pat.items[i]);
		
	}
	return equal;
}


bool hc_string_cmp_cstr(hc_String s, const char* str) {
    return (s.count == strlen(str)) && strncmp(s.items, str, s.count) == 0;
}

bool hc_string_cmp(hc_String l, hc_String r) {
	bool equal = true;
	if(l.count == r.count) {
		for (uint i = 0; i < l.count; i++)
			equal = equal && (l.items[i] == r.items[i]);
	} 
	else 
		return false;
	return equal;
}

bool hc_string_is_empty(hc_String s) {
	return (s.items == NULL || s.count == 0);
}

int hc_string_seek_pattern(hc_String src, hc_String pat) {
	assert(src.count != 0 && pat.count != 0 && 
			"Source and pattern do not allow count of 0");

	assert(src.count > pat.count && 
			"Pattern string cannot be "
			"equal or bigger to Source!"
			"Check hc_string_are_equal(hc_String,hc_String)"
			);

	// src: [ h e l l o ! ] : count 6
	// pat: [ o ! ]  		: count 2
	
	//   0 1 2 3 4
	//   | | | | |
	//   v v v v v
	// [ h e l l o ! ]
	//         [ o ! ]
	//  last index to check is inclusive src.count - pat.count

	size_t diff = src.count - pat.count; 

	if (pat.count == 1){
		for(uint i = 0; i < src.count; i++) {
			if (src.items[i]==pat.items[0])
				return true;
		}
	}
    else {
        for(uint src_i = 0; src_i <= diff; src_i++) {
            bool equal = true;
            for(uint c = 0; c < pat.count; c++)
                equal = equal && (src.items[src_i+c]==pat.items[c]);
            if (equal)
                return src_i;
        }
    }
	return STR_NOPATTERN;
}

bool hc_string_is_integer(hc_String s) {
	bool is_a_num = true;
    if (!s.count) return false;
	for(uint i = 0; i < s.count; i++)
		is_a_num = is_a_num && (
			(s.items[i] >= '0' &&  s.items[i] <= '9')
			||	s.items[0] == '-'
		);
	return is_a_num;
}

bool hc_string_is_float(hc_String s) {
	bool is_a_num = true;
    if (!s.count) return false;
	for(uint i = 0; i < s.count; i++)
		is_a_num = is_a_num && (
			(s.items[i] >= '0' &&  s.items[i] <= '9')
			||	s.items[0] == '-' 
			||	s.items[i] == '.'
		);
	return is_a_num;
}


char* hc_string_create_cstr(hc_String s) {
	char* temp = calloc( (s.count+1)	,	sizeof(char));
	for(uint i = 0; i < s.count+1; i++)
		temp[i] = 0;
	memcpy(temp,s.items,s.count);
	return temp;
}

const char* hc_string_temp_cstr(hc_String s) {
	static char buffer[STR_TEMP_SIZE];
	memset(buffer,0,STR_TEMP_SIZE);
	size_t count = (s.count < STR_TEMP_SIZE - 1) ? 
		s.count : STR_TEMP_SIZE - 1;
	strncpy(buffer,s.items,count);
	return (const char*)buffer;
}

// returns left part of split, source gets reduced
// if first matched character is in chars, returns left as hc_string_zero(void)
// when soucre is empty does nothing.
hc_String hc_string_split_by_chars(hc_String *s, const char* chars) {
    hc_String r = {0};
    size_t chars_count = strlen(chars);
    if (!s->count) return r;
    r.items = s->items;
    size_t count = s->count;
    for(size_t i = 0; i < count; i++) {
        for(size_t lc = 0; lc < chars_count; lc++) {
            const bool match = *(s->items) == chars[lc];
            if(match) {
                s->items++;
                s->count--;
                if(!i) { 
                    return hc_string_zero();
                } 
                return r;
            }
        }
        s->items++;
        s->count--;
        r.count++;
    }
    s->count = 0;
    return r;
}

hc_String hc_string_trim_right(hc_String s) {
    hc_String r = {
        .items = s.items,
        .count = s.count
    };
    for(int i = s.count-1; i >= 0; i--) {
        bool is_space = 
            r.items[i] == '\n' ||
            r.items[i] == '\t' ||
            r.items[i] == '\r' ||
            r.items[i] == ' '  ;
        if (!is_space) return r;
        r.count--;
    }
    return r;
}

hc_String hc_string_trim_left(hc_String s) {
    hc_String r = {
        .items = s.items,
        .count = s.count,
    };
    for(size_t i = 0; i < s.count; i++) {
        bool is_space = 
            *r.items == '\n' ||
            *r.items == '\t' ||
            *r.items == '\r' ||
            *r.items == ' '  ;
        if(!is_space) return r;
        r.count--;
        r.items++;
    }
    return r;
}

hc_String hc_string_trim(hc_String s) {
    return hc_string_trim_left(hc_string_trim_right(s));
}


void hc_string_print_fmt(hc_String s, char* pref, char* pofx) {
	if (pref) printf("%s",pref);
	hc_string_loop(i,s.count) {
		printf("%c",s.items[i]);
	}
	if (pofx) printf("%s",pofx);
}

HC_HINT_INLINE void hc_string_reset(hc_String* s) {
	s->count = 0;
	s->items = NULL;
}


/****************
 ****************
 * String builder 
 ****************
 ****************/

#ifndef __HCH_SB_H
#define __HCH_SB_H

#ifndef __ALLOCATOR_INTERFACE
#   error "String builder requires allocator interface."
#endif

typedef struct {
    // transmutable -> DA , String
    char*       items;
    size_t      count, capacity;
    const char* spacer;
} StringBuilder;

#ifndef STRING_BUILDER_INITIAL_CAPACITY
#   define STRING_BUILDER_INITIAL_CAPACITY 32
#endif

#define stringb_arrlit(...)          ((const char*[]) {__VA_ARGS__})
#define stringb_arrlen(arr)          (sizeof(arr) / sizeof((arr)[0]))
#define stringb_arrlit_len(...)      (stringb_arrlen((__VA_ARGS__)))

#define stringb_append(allocator, sb, ...) \
    stringb__append(allocator,\
            sb,\
            stringb_arrlit(__VA_ARGS__),\
            stringb_arrlen(stringb_arrlit(__VA_ARGS__)))\

#define stringb_min(a,b) ((a) > (b))? (b) : (a)
#define stringb_max(a,b) ((a) < (b))? (b) : (a)

#ifndef __HC_MEMORY_H
void* recalloc(void* ptr, size_t prev_size, size_t size) {
    void* new_ptr = calloc(size, 1);
    if(ptr) {
        memcpy(new_ptr, ptr, prev_size);
        free(ptr);
    }
    return new_ptr;
}
#endif

bool stringb_is_empty(StringBuilder sb) {
    return !sb.items || !sb.items;
}

void stringb__append(Allocator allocator, StringBuilder* sb, const char** items, size_t count) {
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
        sb->capacity = STRING_BUILDER_INITIAL_CAPACITY;
        sb->items = allocator_alloc(allocator, sb->capacity);
    }
    if (sb->count + append_size >= sb->capacity) {
        size_t new_size = stringb_max(sb->capacity*2, sb->capacity + append_size);
        sb->items = allocator_realloc(allocator, sb->items, sb->capacity, new_size);
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

void stringb_reverse(StringBuilder* s) {
	if (stringb_is_empty(*s))
		return;

	char* ptr_cpy = calloc(s->count, sizeof(char));
	if (!ptr_cpy) assert(false && "Failed to allocate" "memory with calloc(n,s)"); 
    memcpy(ptr_cpy, s->items, s->count);
	// [ h i ! ] : len 3
	//   i i i
	//   0 1 2
	//     ^ ^
	//	   | (end) = (len - 1)
	//	   |
	//	   +-> (end) - i

	for(size_t i = 0; i < s->count; i++) {
		size_t reverse = (s->count-1) - i;
		s->items[i] = ptr_cpy[reverse];
	}
	free(ptr_cpy);
}

void stringb_appendf(Allocator allocator, StringBuilder* sb, const char* fmt, ...) {
    va_list args, args_len;
    va_start(args, fmt);
    va_copy(args_len, args);
    size_t size = vsnprintf(0,0,fmt,args_len);
    va_end(args_len);

    const char* temp = calloc(size+1,1);
    vsnprintf((char*)temp, size+1, fmt, args);
    stringb__append(allocator, sb, &temp, 1);
    free((void*)temp);
    
    va_end(args);
}

void stringb_clear(StringBuilder* sb) {
    memset(sb->items, 0, sb->count);
    sb->count = 0;
}

void stringb_drop(Allocator allocator, StringBuilder* sb) {
    
    if(allocator.free)
        allocator_free(allocator, sb->items, sb->capacity);
    memset(sb, 0, sizeof(*sb));

}


#endif//__HCH_SB_H

#endif // __HC_STRING_H
