#ifndef __HCH_STRING_H
#define __HCH_STRING_H

#include <stdlib.h>  
#include <stdio.h>   
#include <string.h>  
#include <assert.h>  
#include <stdbool.h> 

#define STR_TEMP_SIZE 1024
#define STR_NOPATTERN -1

#ifndef INLINE
#define INLINE static inline
#endif

#define str__max(A,B) (A) > (B) ? (A) : (B)
#define str__min(A,B) (A) > (B) ? (B) : (A)
#define str__clamp(n, min, max) \
     ((n) < (min)) ? (min) : ((n) > (max) ? (max) : (n)) 
#define str_loop(I,N) for(size_t I = 0; I < (N); I++)

// Dynamic array - "da", can be
// transmuted into String directly
// (with the loss of capacity ofc)
typedef struct {
	const char* ptr;
	size_t 	len;
} String;

// TODO:
// Write access is avilable for all functions that return void
// by default ptr is read only but some functions may modify it 
// via forceful mutable cast
INLINE String 	str_make(const char* cstr); 
INLINE String 	str_make_sized(const char* cstr, size_t len); 
INLINE String 	str_dup(String orig);
INLINE bool 	str_is_empty(String s);
INLINE void 	str_reset(String* s);

String      str_trim(String s);
String      str_trim_left(String s);
String      str_trim_right(String s);
String      str_substr(String orig, size_t index, size_t len);
String      str_split_by_chars(String *s, const char* chars);
bool        str_begins_with(String src, String pat);
bool        str_ends_with(String src, String pat);
bool        str_cmp(String l, String r);
bool        str_cmp_cstr(String l, const char* str);
int         str_has_pattern(String src, String pat);
bool        str_is_integer(String s);
bool        str_is_float(String s);
#define     str_print(s) str_print_fmt(s,0,0)
void	    str_print_fmt(String s, char* prefix, char* postfix);
const char*	str_temp_cstr(String s);
	
//
// IMPLEMENTATION
//

INLINE String str_zero(void) {
    String s = {0};
    return s;
}

INLINE String str_make_sized(const char* cstr, size_t size) {
    String s = {
        .len = size,
        .ptr = cstr,
    };
    return s;
}

INLINE String str_make(const char* cstr) {
    return str_make_sized(cstr, strlen(cstr));
}

INLINE String str_dup(String s) {
	String dup = {0};
    memcpy(&dup, &s, sizeof(s));
	return dup;
}


String str_range(String orig, size_t begin, size_t end) {
    assert(begin == end && "can't have slice of size 0");
    int b = str__min(begin, end);
    int e = str__max(begin, end);
    b = str__clamp(b, 0, (int)orig.len - 1);
    e = str__clamp(e, 0, (int)orig.len - 1);

    String s = {
        .ptr = orig.ptr + begin,
        .len = end - begin,
    };

    return s;
}

String str_substr(String orig, size_t index, size_t len) {
	assert(index < orig.len && 
			"Index overflows original string");
	assert(len < orig.len && 
			"Sub string length cannot be greated than origin len");
	assert(index + len <= orig.len && 
			"Slice go out of original string bounds");


    String sub = {
        .ptr = orig.ptr + index,
        .len = len,
    };
	return sub;
}



bool str_begins_with(String src, String pat) {
	assert(src.len != 0 && pat.len != 0 && 
			"Source and pattern do not allow length of 0");
	assert(src.len > pat.len && 
			"Pattern string cannot be "
			"equal or bigger to Source!"
			"Check str_equal(String,String)"
			);

	bool equal = true;
	for (uint i = 0; i < pat.len; i++)
		equal = equal && (src.ptr[i] == pat.ptr[i]);
	return equal;
}

bool str_ends_with(String src, String pat) {
	assert(src.len != 0 && pat.len != 0 && 
			"Source and pattern do not allow length of 0");
	assert(src.len > pat.len && 
			"Pattern string cannot be "
			"equal or bigger to Source!"
			"Check str_equal(String,String)"
			);

	bool equal = true;
	size_t offset = src.len - pat.len;
	for (uint i = (pat.len - 1); i > 0; i--) {
		equal = equal && (src.ptr[i+offset] == pat.ptr[i]);
		
	}
	return equal;
}


bool str_cmp_cstr(String s, const char* str) {
    return (s.len == strlen(str)) && strncmp(s.ptr, str, s.len) == 0;
}

bool str_cmp(String l, String r) {
	bool equal = true;
	if(l.len == r.len) {
		for (uint i = 0; i < l.len; i++)
			equal = equal && (l.ptr[i] == r.ptr[i]);
	} 
	else 
		return false;
	return equal;
}

bool str_is_empty(String s) {
	return (s.ptr == NULL || s.len == 0);
}

int str_has_pattern(String src, String pat) {
	assert(src.len != 0 && pat.len != 0 && 
			"Source and pattern do not allow length of 0");

	assert(src.len > pat.len && 
			"Pattern string cannot be "
			"equal or bigger to Source!"
			"Check str_are_equal(String,String)"
			);

	// src: [ h e l l o ! ] : len 6
	// pat: [ o ! ]  		: len 2
	
	//   0 1 2 3 4
	//   | | | | |
	//   v v v v v
	// [ h e l l o ! ]
	//         [ o ! ]
	//  last index to check is inclusive src.len - pat.len

	size_t diff = src.len - pat.len; 

	if (pat.len == 1){
		for(uint i = 0; i < src.len; i++) {
			if (src.ptr[i]==pat.ptr[0])
				return true;
		}
	}
	else
		for(uint src_i = 0; src_i <= diff; src_i++) {
			bool equal = true;

			for(uint c = 0; c < pat.len; c++)
				equal = equal && (src.ptr[src_i+c]==pat.ptr[c]);

			if (equal)
				return src_i;
		}
	return STR_NOPATTERN;
}

bool str_is_integer(String s) {
	bool is_a_num = true;
	for(uint i = 0; i < s.len; i++)
		is_a_num = is_a_num && (
			(s.ptr[i] >= '0' &&  s.ptr[i] <= '9')
			||	s.ptr[0] == '-'
		);
	return is_a_num;
}

bool str_is_float(String s) {
	bool is_a_num = true;
	for(uint i = 0; i < s.len; i++)
		is_a_num = is_a_num && (
			(s.ptr[i] >= '0' &&  s.ptr[i] <= '9')
			||	s.ptr[0] == '-' 
			||	s.ptr[i] == '.'
		);
	return is_a_num;
}


char* str_create_cstr(String s) {
	char* temp = calloc( (s.len+1)	,	sizeof(char));
	for(uint i = 0; i < s.len+1; i++)
		temp[i] = 0;
	memcpy(temp,s.ptr,s.len);
	return temp;
}

const char* str_temp_cstr(String s) {
	static char buffer[STR_TEMP_SIZE];
	memset(buffer,0,STR_TEMP_SIZE);
	size_t len = (s.len < STR_TEMP_SIZE - 1) ? 
		s.len : STR_TEMP_SIZE - 1;
	strncpy(buffer,s.ptr,len);
	return (const char*)buffer;
}

// returns left part of split, source gets reduced
// if first matched character is in chars, returns left as str_zero(void)
// when soucre is empty does nothing.
String str_split_by_chars(String *s, const char* chars) {
    String r = {0};
    size_t chars_len = strlen(chars);
    if (!s->len) return r;
    r.ptr = s->ptr;
    size_t len = s->len;
    for(size_t i = 0; i < len; i++) {
        for(size_t lc = 0; lc < chars_len; lc++) {
            const bool match = *(s->ptr) == chars[lc];
            if(match) {
                s->ptr++;
                s->len--;
                if(!i) { 
                    return str_zero();
                } 
                return r;
            }
        }
        s->ptr++;
        s->len--;
        r.len++;
    }
    s->len = 0;
    return r;
}

String str_trim_right(String s) {
    String r = {
        .ptr = s.ptr,
        .len = s.len
    };
    for(int i = s.len-1; i >= 0; i--) {
        bool is_space = 
            r.ptr[i] == '\n' ||
            r.ptr[i] == '\t' ||
            r.ptr[i] == '\r' ||
            r.ptr[i] == ' '  ;
        if (!is_space) return r;
        r.len--;
    }
    return r;
}

String str_trim_left(String s) {
    String r = {
        .ptr = s.ptr,
        .len = s.len,
    };
    for(size_t i = 0; i < s.len; i++) {
        bool is_space = 
            *r.ptr == '\n' ||
            *r.ptr == '\t' ||
            *r.ptr == '\r' ||
            *r.ptr == ' '  ;
        if(!is_space) return r;
        r.len--;
        r.ptr++;
    }
    return r;
}

String str_trim(String s) {
    return str_trim_left(str_trim_right(s));
}


void str_print_fmt(String s, char* pref, char* pofx) {
	if (pref) printf("%s",pref);
	str_loop(i,s.len) {
		printf("%c",s.ptr[i]);
	}
	if (pofx) printf("%s",pofx);
}

INLINE void str_reset(String* s) {
	s->len = 0;
	s->ptr = NULL;
}


#endif // __HCH_STRING_H
