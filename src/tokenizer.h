#pragma once

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

typedef enum {
	Token_None,
	Token_Other,
	Token_EOL,
	Token_Word,
	Token_StringLiterall,
	Token_IntegerLiterall,
	Token_FloatLiterall,
} TokenType;

typedef struct {
	char* 		text;
	char* 		tag;
} TokenDefinition;

typedef struct {
	char* text;
	size_t len;
	char* tag;
	TokenType type;
} Token;

typedef struct {
	size_t src_len;
	char*  source;

	size_t position;

	size_t token_count;
	Token* tokens;
} Tokenizer;

// API FUNCTIONS
char* tkn_alloc_str(char* cstr);
char tkn_src_curr(Tokenizer  t);
char tkn_src_next(Tokenizer* t);
char tkn_src_prev(Tokenizer t);
void  tkn_append(Token* t, char c);
void tkn_pop(Token* t) ;
void tkn_clear(Token *t) ;
void tkn_tokens_append(Tokenizer *t, Token tok) ;
void tokenizer_clear(Tokenizer *t) ;
void tkn_cstr(Token* t, char* cstr) ;
bool tkn_char_is_newl(char c) ;
bool tkn_char_is_space(char c) ;
bool tkn_char_is_dec(char c) ;
bool tkn_char_is_lowercase_latter(char c) ;
bool tkn_char_is_capitalcase_latter(char c) ;
Token tkn_word(Tokenizer* t) ;
Token tkn_number_literall(Tokenizer* t) ;
Token tkn_string_literall(Tokenizer* t, char quote) ;


// USER LAND
Token tkn_tokenize_from_dictionary
			(Tokenizer* t, 
			 TokenDefinition* dictionary,
			 size_t dict_len);

void tokenize(Tokenizer* t, 
			  TokenDefinition* dict, 
			  size_t dict_len,
			  char quote);



#ifdef HCH_TOKENIZER_IMPLEMENTATION

char* tkn_alloc_str(char* cstr) {
	char* str = calloc(strlen(cstr)+1,sizeof(char));
	strncpy(str,cstr,strlen(cstr));
	return str;
}

char tkn_src_curr(Tokenizer  t) {
	if (t.position < t.src_len) {
		return t.source[t.position];
	}
	return 0;
}

char tkn_src_next(Tokenizer* t) {
	if (t->position < t->src_len) {
		return t->source[t->position+1];
	}
	return 0;
}

char tkn_src_prev(Tokenizer t) {
	if (t.position > 0) {
		return t.source[t.position-1];
	}
	return 0;
}

void  tkn_append(Token* t, char c) {
	t->text = reallocarray(
			t->text,
			t->len+1,
			sizeof(char)
	);
	assert(t->text && "t->text should never be NULL");
	t->text[t->len] = c;
	t->len++;
}

void tkn_pop(Token* t) {
	t->text = reallocarray(
			t->text,
			t->len-1,
			sizeof(char)
	);
	assert(t->text && "t->text should never be NULL");
	t->len--;
}

void tkn_clear(Token *t) {
	if (t->text) {
		free(t->text);
	}
	if (t->tag) {
		free(t->tag);
	}
	t->type = Token_None;
	t->len = 0;
}

void tkn_tokens_append(Tokenizer *t, Token tok) {
	t->tokens = reallocarray(
			t->tokens,
			t->token_count+1,
			sizeof(Token)
		);
	assert(t->tokens && "t->tokens should never be NULL");
	t->tokens[t->token_count] = tok;
	t->token_count++;
}

void tokenizer_clear(Tokenizer *t) {
	if (t->tokens) {
		for (uint i = 0; i < t->token_count; i++) {
			tkn_clear(&t->tokens[i]);
		}
		free(t->tokens);
	}
}

void tkn_cstr(Token* t, char* cstr) {
	for(uint i = 0; i < strlen(cstr);i++){
		tkn_append(t,cstr[i]);
	}
}


bool tkn_char_is_newl(char c) {
	return (c == '\n');
}

bool tkn_char_is_space(char c) {
	return (c == ' ' || c == '\t' );
}

bool tkn_char_is_dec(char c) {
	return (c >= '0' && c <= '9' );
}

bool tkn_char_is_lowercase_latter(char c) {
	return (c >= 'a' && c <= 'z' );
}

bool tkn_char_is_capitalcase_latter(char c) {
	return (c >= 'A' && c <= 'Z' );
}

Token tkn_word(Tokenizer* t) {
	Token tok = {0};
	size_t relative_i = 0;

	for (uint i = t->position; i < t->src_len; i++) {
		t->position = i;
		char curr = tkn_src_curr(*t);
		bool llat   = tkn_char_is_lowercase_latter(curr);
		bool clat   = tkn_char_is_capitalcase_latter(curr);
		bool is_dec = tkn_char_is_dec(curr);

		if (llat || clat || curr=='_') {
			tkn_append(&tok,curr);
		} else if (is_dec && relative_i != 0) {
			tkn_append(&tok,curr);
		}
		else {

			tok.type = Token_Word;
			break;
		}

		relative_i++;
	}
	return tok;
}

Token tkn_number_literall(Tokenizer* t) {
	Token tok = {0};
	char curr = tkn_src_curr(*t);
	char prev = 0;
	char next = 0;
	uint relative_i = 0;
	uint dot_count  = 0;

	for (uint i = t->position; i < t->src_len; i++) {
		t->position = i;
		curr = tkn_src_curr(*t);
		prev = tkn_src_prev(*t);
		next = tkn_src_next(t);

		if (tkn_char_is_dec(curr)) {
			tkn_append(&tok,curr);
		} else if (	curr == '.' &&
					tkn_char_is_dec(next) &&
					relative_i != 0 && 
					dot_count < 1) 
		{
			tok.type = Token_FloatLiterall;
			tkn_append(&tok,curr);
			dot_count++;
		}
		else  {
			break;
		}

		relative_i++;
	}

	if (tok.type != Token_FloatLiterall &&
		tok.len > 0) 
	{
		tok.type = Token_IntegerLiterall; 
	}

	return tok;
}


Token tkn_string_literall(Tokenizer* t, char quote) {
	Token tok = {0};
	char curr = tkn_src_curr(*t);
	char prev = 0;//tkn_src_prev(*t);
	char next = 0;//tkn_src_next(t);

	if (curr == quote) {
		t->position++;
		//printf(" HIT ");
	}
	else {
		return tok; // EMPTY
	}

	for(uint i = t->position; i < t->src_len; i++) {
		t->position = i;
		curr = tkn_src_curr(*t);
		prev = tkn_src_prev(*t);
		next = tkn_src_next(t);
		
		if (curr == quote)
		{
			if (prev == '\\') {
				tkn_pop(&tok);
				tkn_append(&tok,quote);
				continue;
			}
			else {
				// we have to skip one char forward
				// to prevent double quote hits
				tok.type = Token_StringLiterall;
				t->position += 1;
				break;
			}
		}
		else {
			tkn_append(&tok, t->source[i]);
		}
	}
	return tok;
}

Token tkn_tokenize_from_dictionary
	(Tokenizer* t, 
	 TokenDefinition* dictionary,
	 size_t dict_len) 
{
	Token tok = {0};

	for(uint i = 0; i < dict_len; i++) {
		TokenDefinition tok_def = dictionary[i];
		size_t tok_len = strlen(tok_def.text);
		bool fits_src = (t->position < t->src_len);
		size_t src_rem = 0;
		char* current = 0;
		
		if (fits_src) {
			current = t->source + t->position;
			src_rem = strlen(current);
		} else continue;

		if (src_rem >= tok_len && 
			strncmp(tok_def.text,current,tok_len) == 0) 
		{
			tkn_cstr(&tok,tok_def.text);
			t->position += tok_len;
			tok.tag = tkn_alloc_str(tok_def.tag);
			tok.type = Token_Other;
			break;
		} else continue;

	}

	return tok;
}

void tokenize
	(Tokenizer* t, 
	 TokenDefinition* dict, 
	 size_t dict_len,
	 char quote) 
{
	Token tok = {0};

	for(uint i = 0; i < t->src_len; i++) {
	
		char ch = tkn_src_curr(*t);
		bool is_uppercase = tkn_char_is_capitalcase_latter(ch);
		bool is_lowercase = tkn_char_is_lowercase_latter(ch);
		bool is_decimal   = tkn_char_is_dec(ch);
		bool is_space     = tkn_char_is_space(ch);
		bool is_newline   = tkn_char_is_newl(ch);

		if (is_space || is_newline) {
			t->position+=1;

			if (is_newline) {
				tok.text = NULL;
				tok.tag  = NULL;
				tok.type = Token_EOL;
				tkn_tokens_append(t,tok);
			}

			continue;

		} else if (ch == quote) {
			tok = tkn_string_literall(t,quote);

		} else if (is_uppercase || is_lowercase) {
			if (t->source[t->position+1] == 0) {
				return;
			}
			tok = tkn_word(t);

		} else if (is_decimal) {
			if (t->source[t->position+1] == 0) {
				return;
			}
			tok = tkn_number_literall(t);

		} else {
			tok = tkn_tokenize_from_dictionary(t,dict,dict_len);
		}

		if (tok.len > 0) {
			tkn_tokens_append(t,tok);

		} else {
			t->position++;
			tkn_clear(&tok);
		}

		if (t->position >= t->src_len)
			break;
	}

}

#endif
