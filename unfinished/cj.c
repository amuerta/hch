#define HCH_TOKENIZER_IMPLEMENTATION
#include "tokenizer.h"
#include <stdio.h>

typedef struct {
	Tokenizer* tokenizer;
	size_t 	   position;
} Parser;

typedef enum {
	CJ_bool,
	CJ_int,
	CJ_float,
	CJ_string,
	CJ_object,
} CJ_Type;

typedef struct CJ_Object {
	CJ_Type 		  data_type;
	char* 			  var_name;
	union {
		void* 				as_null;
		bool				as_bool;
		int					as_int;
		float				as_float;
		char* 				as_string;
		struct CJ_Object* 	as_object;
	} var_value;

	size_t 					child_count;
	struct CJ_Object* 		children;
} CJ_Object;


CJ_Object* CJ_init(char* var_name, CJ_Type type) {
	CJ_Object* obj = {0};
	
	obj = calloc(
			1,
			sizeof(CJ_Object)
	);
	assert(obj && "Failed to allocate object");
	
	obj->var_name = calloc(
			strlen(var_name)+1,
			sizeof(char)
	);
	assert(obj && "Failed to allocate variable_name");
	
	strcpy(obj->var_name, var_name);
	obj->data_type = type;
	return obj;
}

void CJ_append_children(CJ_Object *obj, CJ_Object item) {
	obj->children = reallocarray(
			obj->children,
			obj->child_count + 1,
			sizeof(CJ_Object)
	);
	assert(obj && "Failed to reallocate object");
	obj->children[obj->child_count] = item;
	obj->child_count++;
}

bool CJ_is_field(Parser *p) {
	Tokenizer* t = p->tokenizer;
	bool fits = t->token_count >= p->position + 3;

	bool has_word = fits && 
			t->tokens[p->position  ].type == Token_Word;
	bool has_def = fits && 
			strcmp(
				t->tokens[p->position+1].tag,"var_def"
			) == 0;
	bool has_value = fits && (
			 t->tokens[p->position+2].type == Token_StringLiterall ||
			 t->tokens[p->position+2].type == Token_IntegerLiterall ||
			 t->tokens[p->position+2].type == Token_FloatLiterall ||

			 strcmp(
				 t->tokens[p->position+2].tag,"obj_open"
				 ) == 0 										|| 

			 strcmp(
				 t->tokens[p->position+2].tag,"arr_open"
				 ) == 0 										 
			);


	printf("word: %d\t def: %d\t value: %d\n",has_word,has_def,has_value);
	return (has_word && has_def && has_value);
}

const TokenDefinition CJ_TOKEN_DICTIONARY[] = {
	{":",  	"var_def"	},
	{",",  	"var_delim"	},
	{"{", 	"obj_open"	},
	{"}", 	"obj_close"	},
	{"[", 	"arr_open"	},
	{"]", 	"arr_close"	},
	//{},
};
const size_t CJ_DICT_SIZE = sizeof(CJ_TOKEN_DICTIONARY) / sizeof(CJ_TOKEN_DICTIONARY[0]);

CJ_Object* CJ_parse_string(char* string) {
	Tokenizer t = {
		.source = string,
		.src_len = strlen(string),
		.position = 0
	};

	Parser p = {
		.tokenizer 	= &t,
		.position 	= 0
	};

	tokenize(&t,(TokenDefinition*) CJ_TOKEN_DICTIONARY,CJ_DICT_SIZE,'\'');


	if (CJ_is_field(&p)) {
		printf("\tyes\n");
	} else {
		printf("len: %lu\n",t.token_count);
	}

	for (uint i = 0; i < t.token_count; i++) {
		Token tk = t.tokens[i];
		printf("<");
		for(uint c = 0; c < tk.len; c++) {
			printf("%c",tk.text[c]);
		}
		printf(">:%i\t",(int)tk.type);
	}


	tokenizer_clear(&t);
	return NULL;
}


int main(void) {
	char* string = " var: 12 ";
	CJ_parse_string(string);
}
