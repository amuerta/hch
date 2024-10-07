#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef struct {
	uint 	id;

	struct Tree* parent;
	struct Tree* children;
	size_t children_count;
	size_t children_capacity;
	
	enum { Root, Branch, Leaf } node_type;
	void* 	data;
} Tree;


