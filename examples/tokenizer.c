#define HCH_TOKENIZER_IMPLEMENTATION
#include "tokenizer.h"
#include <stdio.h>

int main(int argc, char** argv) {
	char* string = "var = 'var' + 1.2 + vas \n full";

	Tokenizer t = {
		.source = string,
		.src_len = strlen(string),
		.position = 0
	};

	TokenDefinition dict[] = {
		{"\n",  "EOL"},
		{"-",  "BinOp"},
		{"+",  "BinOp"},
		{"=",  "BinOp"},
		{";",  "BinOp"},
	};

	tokenize(&t,dict,5,'\'');

	for (uint i = 0; i < t.token_count; i++) {
		Token tk = t.tokens[i];
		printf("{");
		for(uint c = 0; c < tk.len; c++) {
			if (!tk.text) {
				printf("newl");
				break;
			}
			printf("%c",tk.text[c]);

		}
		printf("}\t");
	}
	tokenizer_clear(&t);
}
