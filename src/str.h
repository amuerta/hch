/*
	MIT No Attribution

	Copyright <2024> (amuerta) <amurtasmail@gmail.com>

	Permission is hereby granted, free of charge, to any person obtaining a copy of this
	software and associated documentation files (the "Software"), to deal in the Software
	without restriction, including without limitation the rights to use, copy, modify,
	merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
	permit persons to whom the Software is furnished to do so.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
	INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
	PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
	HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
	OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
	SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once
#include <stdlib.h> 
// - size_t
// - calloc
#include <stdio.h>
// - printf
#include <string.h>
// - memcpy 
#include <assert.h>
// - assert
#include <stdbool.h>
// - #define {true, false} {1,0}

#define FULL_LENGTH 0
#define STR_NOPATTERN -1

typedef struct {
	char* 	ptr;
	size_t 	len;
} String;

String str_from_cstr(char* cstr, size_t len); 
	// creates a string view from cstr, creates a copy of cstr,
	// allocates memory!
String str_move_cstr(char* cstr);
	// creates a string view without ownership of cstr,
	// does not allocate memory!
String str_create(char* cstr);
	// shorten version of str_from_cstr(char*,size_t);
	// allocated memory!
String str_dup(String orig);
	// creates a copy of existing String
	// allocates memory!
String str_substr(String orig, size_t index, size_t len);
	// creates a new string from origin begining from index
	// and finishing at index+len
	// if index or len go out of bounds
	// asserts error
void str_append_chars(String *s,char* chars);
	// appends any N of chars to the end of 
	// the String s
void str_append_char(String *s, char ch);
	// appends single char to String s
	// NOTE:
	//		Highly inefficient due O(N) of reallocations
	//		Better use str_append_chars(String,char*) with 
	//		fixed size char buffer.
void str_reverse(String* s);
	// reverses the string characters
bool str_begins_with(String src, String pat);
	// returns true if pattern is the begining of source
	// asserts any of strings len is 0 or 
	// when pattern is bigger then source
bool str_ends_with(String src, String pat);
	// same as str_begins_with(String,String) 
	// but checks the end of the string instead
	// asserts follow the same rules as in a
	// function above
bool str_are_equal(String l, String r);
	// returns true if strings are equal
bool str_is_empty(String s);
	// returns true if s.len == 0 or when 
	// string pointer points to NULL
int str_has_pattern(String src, String pat);
	// iterates over string source to find 
	// substring defined in pattern,
	// returns begining index of pattern on sucess
	// -1 or STR_NOPATTERN on failure
bool str_is_integer(String s);
	// returns true if all the characters
	// are either numbers or a minus sign
bool str_is_float(String s);
	// same as str_is_integer(String)
	// but also checks for a dot: '.'
char* str_get_cstr(String s);
	// returns copied string pointer terminated by 0
	// needs to be freed after seperately
	// used for printing and <string.h> functions
void str_clear(String* s);
	// resets string contents
	// frees whatevet was under the pointer


//
// IMPLEMENTATION
//

#ifdef STR_IMPLEMENTATION

String str_from_cstr(char* cstr, size_t len) {
	
	assert(strlen(cstr)>=len && 
			"String length cannot be greater"
			" than provided cstr len"		 );

	// if len set as 0, use entire string length
	if (len == 0)
		len = strlen(cstr);

	String s = {0};
	s.len = len;
	s.ptr = calloc(len,sizeof(char));

	if (!s.ptr) assert(false && 
			"Failed to allocate"
			"memory with calloc(n,s)"
		);

	memcpy(s.ptr,cstr,len);
	return s;
}

String str_create(char* cstr) {
	return str_from_cstr(cstr,FULL_LENGTH);
}

String str_move_cstr(char* cstr) {
	String s = {
		.len = strlen(cstr),
		.ptr = cstr,
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

	if (!dup.ptr)
		assert(false && 
				"Failed to allocate"
				"memory with calloc(n,s)");
	memcpy(dup.ptr,s.ptr,s.len);
	return dup;
}


String str_substr(String orig, size_t index, size_t len) {
	assert(index < orig.len && 
			"Index overflows original string");
	assert(len < orig.len && 
			"Sub string length cannot be greated than origin len");
	assert(index + len <= orig.len && 
			"Slice go out of original string bounds");


	// alloca is buggy, fix-sized array is tedious
	char* buffer = calloc(len+1,sizeof(char)); 
	for(uint i = 0; i < len; i++) 
		buffer[i] = orig.ptr[index+i];
	
	String s = str_create(buffer);
	free(buffer);
	return s;
}

void str_append_chars(String *s,char* chars) {

	size_t chars_len = strlen(chars);
	size_t new_len   = s->len + chars_len;
	
	if (chars_len == 0)
		return;

	if (str_is_empty(*s)) {
		*s = str_create(chars);
		return;
	}

	char* new_ptr = reallocarray(s->ptr,
								 new_len,
								 sizeof(char));
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
	
	if (str_is_empty(*s)) {
		char one_char_cstr[2] = { ch, 0 };
		*s = str_create(one_char_cstr);
		return;
	}

	char* new_ptr = reallocarray(s->ptr,
								 new_len,
								 sizeof(char));
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
	return (s.len == 0 || s.ptr == NULL);
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
			||	s.ptr[i] == '-'
		);
	return is_a_num;
}

bool str_is_float(String s) {
	bool is_a_num = true;
	for(uint i = 0; i < s.len; i++)
		is_a_num = is_a_num && (
			(s.ptr[i] >= '0' &&  s.ptr[i] <= '9')
			||	s.ptr[i] == '-' 
			||	s.ptr[i] == '.'
		);
	return is_a_num;
}


char* str_get_cstr(String s) {
	char* temp = calloc( (s.len+1)	,	sizeof(char));
	for(uint i = 0; i < s.len+1; i++)
		temp[i] = 0;
	memcpy(temp,s.ptr,s.len);
	return temp;
}


void str_clear(String* s) {
	s->len = 0;
	if(s->ptr)
		free(s->ptr);
	s->ptr = NULL;
}

#endif
