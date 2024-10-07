#pragma once

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

#define __da_check(A) \
	assert((A).len >= 0 && "STRUCT DOES NOT CONTAIN LEN FIELD");  	\
	assert((A).items>= 0 && "STRUCT DOES NOT CONTAIN ITEMS FIELD");  \

#define da_append(A,I) do			\
{									\
	__da_check(A)					\
	(A).items = reallocarray( 		\
			(A).items,				\
			(A).len + 1,			\
			sizeof( *((A).items) )  \
	); 							   	\
	if ((A).len == 0)				\
		(A).items[0] = (I); 		\
	else 							\
		(A).items[(A).len] = (I);	\
	(A).len += 1; 					\
} while(0)						

#define da_join(A1,A2) do						\
{												\
	__da_check(A1)								\
	__da_check(A2)								\
	(A1).items = reallocarray(					\
			(A1).items,							\
			(A1).len + (A2).len,				\
			sizeof( *((A1).items) ) 			\
	); 											\
	for(int i = 0; i < (A2).len; i++) { 		\
		int pos = (A1.len); 					\
		(A1).items[pos + i] = ((A2).items[i]);	\
	}											\
	(A1).len += (A2).len;						\
} while(0)

#define da_pop(A) do				\
{									\
	__da_check(A)					\
	assert((A).len > 0 && 			\
			"CANT POP ELEMENT FROM EMPTY ARRAY");						\
	(A).items = reallocarray( 		\
			(A).items,				\
			(A).len - 1,			\
			sizeof( *((A).items) )  \
	); 							   	\
	(A).len -= 1; 					\
} while(0)				

#define da_print(A,FMT) do  			\
{										\
	printf("[ "); 						\
	for(uint i = 0; i < (A).len; i++) 	\
		printf(FMT " ",(A).items[i]); 	\
	printf("]\n");						\
} while(0)

#define da_free(A)  \
	free((A).items)
