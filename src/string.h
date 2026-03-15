#ifndef __HC_STRING_H
#define __HC_STRING_H

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
// transmuted into hc_String directly
// (with the loss of capacity ofc)
typedef struct {
	const char* items;
	size_t 	count;
} hc_String;

// TODO:
// Write access is avilable for all functions that return void
// by default items is read only but some functions may modify it 
// via forceful mutable cast
INLINE hc_String 	str_make(const char* cstr); 
INLINE hc_String 	str_make_sized(const char* cstr, size_t count); 
INLINE hc_String 	str_dup(hc_String orig);
INLINE bool 	str_is_empty(hc_String s);
INLINE void 	str_reset(hc_String* s);

hc_String      str_trim(hc_String s);
hc_String      str_trim_left(hc_String s);
hc_String      str_trim_right(hc_String s);
hc_String      str_substr(hc_String orig, size_t index, size_t count);
hc_String      str_split_by_chars(hc_String *s, const char* chars);
bool        str_begins_with(hc_String src, hc_String pat);
bool        str_ends_with(hc_String src, hc_String pat);
bool        str_cmp(hc_String l, hc_String r);
bool        str_cmp_cstr(hc_String l, const char* str);
int         str_has_pattern(hc_String src, hc_String pat);
bool        str_is_integer(hc_String s);
bool        str_is_float(hc_String s);
#define     str_print(s) str_print_fmt(s,0,0)
void	    str_print_fmt(hc_String s, char* prefix, char* postfix);
const char*	str_temp_cstr(hc_String s);
	
//
// IMPLEMENTATION
//

INLINE hc_String str_zero(void) {
    hc_String s = {0};
    return s;
}

INLINE hc_String str_make_sized(const char* cstr, size_t size) {
    hc_String s = {
        .count = size,
        .items = cstr,
    };
    return s;
}

INLINE hc_String str_make(const char* cstr) {
    return str_make_sized(cstr, strlen(cstr));
}

INLINE hc_String str_dup(hc_String s) {
	hc_String dup = {0};
    memcpy(&dup, &s, sizeof(s));
	return dup;
}


hc_String str_range(hc_String orig, size_t begin, size_t end) {
    assert(begin == end && "can't have slice of size 0");
    int b = str__min(begin, end);
    int e = str__max(begin, end);
    b = str__clamp(b, 0, (int)orig.count - 1);
    e = str__clamp(e, 0, (int)orig.count - 1);

    hc_String s = {
        .items = orig.items + begin,
        .count = end - begin,
    };

    return s;
}

hc_String str_substr(hc_String orig, size_t index, size_t count) {
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



bool str_begins_with(hc_String src, hc_String pat) {
	assert(src.count != 0 && pat.count != 0 && 
			"Source and pattern do not allow countgth of 0");
	assert(src.count > pat.count && 
			"Pattern string cannot be "
			"equal or bigger to Source!"
			"Check str_equal(hc_String,hc_String)"
			);

	bool equal = true;
	for (uint i = 0; i < pat.count; i++)
		equal = equal && (src.items[i] == pat.items[i]);
	return equal;
}

bool str_ends_with(hc_String src, hc_String pat) {
	assert(src.count != 0 && pat.count != 0 && 
			"Source and pattern do not allow countgth of 0");
	assert(src.count > pat.count && 
			"Pattern string cannot be "
			"equal or bigger to Source!"
			"Check str_equal(hc_String,hc_String)"
			);

	bool equal = true;
	size_t offset = src.count - pat.count;
	for (uint i = (pat.count - 1); i > 0; i--) {
		equal = equal && (src.items[i+offset] == pat.items[i]);
		
	}
	return equal;
}


bool str_cmp_cstr(hc_String s, const char* str) {
    return (s.count == strlen(str)) && strncmp(s.items, str, s.count) == 0;
}

bool str_cmp(hc_String l, hc_String r) {
	bool equal = true;
	if(l.count == r.count) {
		for (uint i = 0; i < l.count; i++)
			equal = equal && (l.items[i] == r.items[i]);
	} 
	else 
		return false;
	return equal;
}

bool str_is_empty(hc_String s) {
	return (s.items == NULL || s.count == 0);
}

int str_has_pattern(hc_String src, hc_String pat) {
	assert(src.count != 0 && pat.count != 0 && 
			"Source and pattern do not allow countgth of 0");

	assert(src.count > pat.count && 
			"Pattern string cannot be "
			"equal or bigger to Source!"
			"Check str_are_equal(hc_String,hc_String)"
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
	else
		for(uint src_i = 0; src_i <= diff; src_i++) {
			bool equal = true;

			for(uint c = 0; c < pat.count; c++)
				equal = equal && (src.items[src_i+c]==pat.items[c]);

			if (equal)
				return src_i;
		}
	return STR_NOPATTERN;
}

bool str_is_integer(hc_String s) {
	bool is_a_num = true;
    if (!s.count) return false;
	for(uint i = 0; i < s.count; i++)
		is_a_num = is_a_num && (
			(s.items[i] >= '0' &&  s.items[i] <= '9')
			||	s.items[0] == '-'
		);
	return is_a_num;
}

bool str_is_float(hc_String s) {
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


char* str_create_cstr(hc_String s) {
	char* temp = calloc( (s.count+1)	,	sizeof(char));
	for(uint i = 0; i < s.count+1; i++)
		temp[i] = 0;
	memcpy(temp,s.items,s.count);
	return temp;
}

const char* str_temp_cstr(hc_String s) {
	static char buffer[STR_TEMP_SIZE];
	memset(buffer,0,STR_TEMP_SIZE);
	size_t count = (s.count < STR_TEMP_SIZE - 1) ? 
		s.count : STR_TEMP_SIZE - 1;
	strncpy(buffer,s.items,count);
	return (const char*)buffer;
}

// returns left part of split, source gets reduced
// if first matched character is in chars, returns left as str_zero(void)
// when soucre is empty does nothing.
hc_String str_split_by_chars(hc_String *s, const char* chars) {
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
                    return str_zero();
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

hc_String str_trim_right(hc_String s) {
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

hc_String str_trim_left(hc_String s) {
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

hc_String str_trim(hc_String s) {
    return str_trim_left(str_trim_right(s));
}


void str_print_fmt(hc_String s, char* pref, char* pofx) {
	if (pref) printf("%s",pref);
	str_loop(i,s.count) {
		printf("%c",s.items[i]);
	}
	if (pofx) printf("%s",pofx);
}

INLINE void str_reset(hc_String* s) {
	s->count = 0;
	s->items = NULL;
}


#endif // __HC_STRING_H
