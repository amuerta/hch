#ifndef __HCH_STRING_H
#define __HCH_STRING_H

#include <stdlib.h>  
#include <stdio.h>   
#include <string.h>  
#include <assert.h>  
#include <stdbool.h> 

#define STR_TEMP_SIZE 1024
#define STR_NOPATTERN -1

// Dynamic array - "da", can be
// transmuted into String directly
typedef struct {
	char* 	ptr;
	size_t 	len;
	size_t  cap;
} String;

// TODO:
// Write access is avilable for all functions that return void
// by default ptr is read only but some functions may modify it 
// via forceful mutable cast
String  str_prealloc(size_t cap);
String 	str_from_cstr(char* cstr, size_t len); 
String 	str_move_cstr(char* cstr);
String 	str_create(char* cstr);
String 	str_dup(String orig);
String 	str_substr(String orig, size_t index, size_t len);
String* str_split(String src, char divisor, size_t* count);
void 	str_append_chars(String *s,char* chars);
void 	str_append_char(String *s, char ch);
void 	str_reverse(String* s);
bool 	str_begins_with(String src, String pat);
bool 	str_ends_with(String src, String pat);
bool 	str_are_equal(String l, String r);
bool 	str_is_empty(String s);
int 	str_has_pattern(String src, String pat);
bool 	str_is_integer(String s);
bool 	str_is_float(String s);
#define str_print(s) str_print_fmt(s,0,0)
void	str_print_fmt(String s, char* prefix, char* postfix);

char*	str_temp_cstr(String s);
void 	str_clear(String* s);
void 	str_free(String* s);
	
// TODO: find quicker comparison for sequence of characters
//
#if 0
bool	str_memcmp(char* src, char* cmp, size_t pos, size_t len);
	// compares
bool	str_memncmp(char* src, char* cmp, size_t block_sizes[2], size_t pos, size_t len);
	// compares a block of memory in src at index pos,
#endif

//
// IMPLEMENTATION
//

#define str__max(A,B) (A) > (B) ? (A) : (B)
#define str__min(A,B) (A) > (B) ? (B) : (A)
#define str__clamp(n, min, max) \
     ((n) < (min)) ? (min) : ((n) > (max) ? (max) : (n)) 
#define str_loop(I,N) for(size_t I = 0; I < (N); I++)

String str_zero(void) {
    String s = {0};
    return s;
}

String str_prealloc(size_t cap) {
	return (String) {
		.cap = cap,
		.len = 0,
		.ptr = calloc(cap,sizeof(char)),
	};
}

#define str_set(s, cstr) str_set_sized(s, cstr, strlen(cstr))
void str_set_sized(String* s, char* ptr, size_t len) {
    assert(s && "Expected string to be not NULL");
    if (!ptr || !len) return;

    if(len >= s->cap) {
        if (!s->cap && s->ptr)
            s->ptr = calloc(len, 1);
        else
            s->ptr = realloc(s->ptr, len);
        s->cap = len;
        s->len = len;
    }
    memcpy(s->ptr, ptr, len);
}

String str_from_cstr(char* cstr, size_t len) {
	
	size_t prealloc_count;
	assert(strlen(cstr)>=len && 
			"String length cannot be greater"
			" than provided cstr len"		 );

	// if len set as 0, use entire string length
	if (len == 0)
		len = strlen(cstr);
	
	prealloc_count = len;
#ifdef STR_PREALLOC_BYTES
	prealloc_count = str__max(STR_PREALLOC_BYTES,len);
#endif

	String s = {0};
	s.len = len;
	s.cap = prealloc_count;
	s.ptr = calloc(prealloc_count,sizeof(char));

	if (!s.ptr) assert(false && 
			"Failed to allocate"
			"memory with calloc(n,s)"
		);

	memcpy(s.ptr,cstr,len);
	return s;
}

String str_create(char* cstr) {
	return str_from_cstr(cstr,strlen(cstr));
}

String str_move_cstr(char* cstr) {
	size_t len = strlen(cstr);
	String s = {
		.len = len,
		.ptr = cstr,
		.cap = len,
	};
	return s;
}

String str_dup(String s) {
	String dup = {0};
	
	if (str_is_empty(s))
		return dup;

	assert(s.len != 0 && 
			"String is not reset, but has 0 length");

	dup.ptr = calloc(s.len,	sizeof(char));
	dup.len = s.len;
	dup.cap = s.len;

	if (!dup.ptr)
		assert(false && 
				"Failed to allocate"
				"memory with calloc(n,s)");
	memcpy(dup.ptr,s.ptr,s.len);
	return dup;
}


String str_slice(String orig, size_t begin, size_t end) {
    assert(begin == end && "can't have slices of same size");
    int b = str__min(begin, end);
    int e = str__max(begin, end);
    b = str__clamp(b, 0, (int)orig.len - 1);
    e = str__clamp(e, 0, (int)orig.len - 1);

    String s = {
        .ptr = orig.ptr + begin,
        .len = end - begin,
        .cap = 0
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
        .cap = 0
    };
	return sub;
}

void str_append_chars(String *s, char* chars) {
	char* new_ptr 		= 0;
	size_t chars_len 	= strlen(chars);
	size_t new_len   	= s->len + chars_len;
	size_t realloc_size = s->len == 0 ?
		 1 : s->len * 2;

	if (chars_len == 0)
		return;

	if (str_is_empty(*s)) {
		*s = str_create(chars);
		return;
	}

	if (s->len + strlen(chars) > s->cap) {
		 new_ptr = reallocarray(s->ptr,
				realloc_size,
				sizeof(char));
		 s->cap = realloc_size;
	}

	if (!new_ptr)
		assert(false && 
				"failed to reallocate memory"
				"using reallocarray(ptr,nmemb,size)"
		);

	// [ j i m ]
	// [ j i m m y] <- new_len 3+2
	//   0 1 2
	//         ^  for (i,i<new_len;i++)
	//         |          ~~~~~~~~(4)
	//         len
	size_t diff = new_len - s->len;
	for(uint i = 0; i < diff; i++)
		new_ptr[i+diff] = chars[i];
			//  0 + 2 + 1 = 3
			//  1 + 2 + 1 = 4
	s->ptr = new_ptr;
	s->len = new_len;
}


void str_append_char(String *s, char ch) {
	size_t new_len   = s->len + 1;
	char* new_ptr = s->ptr;
	size_t realloc_size = s->len == 0 ?
		 1 : s->len * 2;
	
	if (str_is_empty(*s)) {
		char one_char_cstr[2] = { ch, 0 };
		*s = str_create(one_char_cstr);
		return;
	}
	if (new_len > s->cap) {
		new_ptr = reallocarray(s->ptr,
				realloc_size,
				sizeof(char));
		s->cap = realloc_size;
	}

	if (!new_ptr)
		assert(false && 
				"failed to reallocate memory"
				"using reallocarray(ptr,nmemb,size)"
		);
	
	new_ptr[s->len] = ch;

	s->ptr = new_ptr;
	s->len = new_len;
}


void str_reverse(String* s) {
	if (str_is_empty(*s))
		return;

	char* ptr_cpy = calloc(s->len,	sizeof(char));
	if (!ptr_cpy)
		assert(false && 
				"Failed to allocate"
				"memory with calloc(n,s)");
	memcpy(ptr_cpy, s->ptr, s->len);
	// [ h i ! ] : len 3
	//   i i i
	//   0 1 2
	//     ^ ^
	//	   | (end) = (len - 1)
	//	   |
	//	   +-> (end) - i

	for(uint i = 0; i < s->len; i++) {
		size_t reverse = (s->len-1) - i;
		s->ptr[i] = ptr_cpy[reverse];
	}
	free(ptr_cpy);
}

bool str_begins_with(String src, String pat) {
	assert(src.len != 0 && pat.len != 0 && 
			"Source and pattern do not allow length of 0");
	assert(src.len > pat.len && 
			"Pattern string cannot be "
			"equal or bigger to Source!"
			"Check str_are_equal(String,String)"
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
			"Check str_are_equal(String,String)"
			);

	bool equal = true;
	size_t offset = src.len - pat.len;
	for (uint i = (pat.len - 1); i > 0; i--) {
		equal = equal && (src.ptr[i+offset] == pat.ptr[i]);
		
	}
	return equal;
}

bool str_are_equal(String l, String r) {
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
	return (s.ptr == NULL);
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

char* str_temp_cstr(String s) {
	static char buffer[STR_TEMP_SIZE];
	memset(buffer,0,STR_TEMP_SIZE);
	size_t len = (s.len < STR_TEMP_SIZE - 1) ? 
		s.len : STR_TEMP_SIZE - 1;
	strncpy(buffer,s.ptr,len);
	return buffer;
}

String str_split_chars(String* s, const char* chars) {
    assert(s);
    if(*s->ptr == 0) *s = str_zero();
    String l = {.ptr=s->ptr,0};
    for(size_t i = 0; i < s->len; i++) {
        for(size_t c = 0; c < strlen(chars); c++)
            if(*s->ptr == chars[c]) {
                s->ptr++;
                if (*s->ptr) s->len = s->len-l.len-1;
                else *s = str_zero();
                return l;
            }
        l.len++;
        s->ptr++;
    }
    return l;
}

// TODO: increase performance?
String* str_split_many(String src, char divisor, size_t* count) {
	assert(src.len > 0 && "source shouldn't be empty");
	assert(count && "count shouldn't be NULL");
	String  item = str_prealloc(32);
	String* items = 0;

	str_loop(i, src.len) {
		bool slice_eq = src.ptr[i] == divisor;
		bool trail_str =  ( !str_is_empty(item) && i == src.len-1);

		if ( slice_eq || trail_str) {
			if (src.len-1 == i) {
				str_append_char(&item,src.ptr[i]);
			}

			const size_t newlen = ((*count)+1) * sizeof(String);
			items = realloc(items, newlen);
			items[*count] = str_dup(item);
			str_clear(&item);
			(*count)++;
		} else {
			str_append_char(&item,src.ptr[i]);
		}

	}

	str_free(&item);
	return items;
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

bool str_cmp_str(String s, const char* str) {
    return (s.len == strlen(str)) && strncmp(s.ptr, str, s.len) == 0;
}

void str_print_fmt(String s, char* pref, char* pofx) {
	if (pref) printf("%s",pref);
	str_loop(i,s.len) {
		printf("%c",s.ptr[i]);
	}
	if (pofx) printf("%s",pofx);
}

void str_clear(String* s) {
	s->len = 0;
    if(s->cap)
        memset(s->ptr,0,s->cap);
}

void str_free(String* s) {
    assert(s->cap && "Can't free slice, only owned strings");
	s->len = 0;
	s->cap = 0;
	if (s->ptr) 
		free(s->ptr);
	s->ptr = NULL;
}


#endif // __HCH_STRING_H
