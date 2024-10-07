#pragma once

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
	int 	argc;
	char** 	argv;
} Args;


int arg_find(Args args,char* arg_literall);
//	will return index of the argument if it exists or 0
char* arg_find_value(Args args,char* arg_literalls);
// 	will return a pointer to string in argv that is 
// 	after the provided argument literall
//
// 	to handle lists use tokinizer function of choise and wrap the 
// 	argument list with double quotes
char* arg_get_path(Args args);
// 	return invocation path of the program 
// 	that is: argv[0]


#ifdef HCH_ARGS_IMPLEMENTATION

Args args_init(int argc, char** argv) {
	return (Args) {
		.argc = argc,
		.argv = argv
	};
}

int arg_find(Args a,char* lit) {
	int ret = 0;
	for (uint i = 1; i < a.argc; i++) 
		if (strcmp(a.argv[i],lit)==0)
			ret = i;
	return ret;
}

char* arg_find_value(Args a, char* lit) {
	char* ret = NULL;
	for (uint i = 1; i < a.argc; i++) {
		bool equal = (strcmp(a.argv[i],lit)==0);
		bool within_args = ( i+1 < a.argc );
		if (equal && within_args) 
			ret = a.argv[i+1];
	}
	return ret;
}

char* arg_get_path(Args a) {
	return a.argv[0];
}

#endif
