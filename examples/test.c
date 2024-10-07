#define HCH_LIST
#define HCH_ARGS_IMPLEMENTATION
#include "list.h"
#include "cliargs.h"
#include "tree.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define list_push(L,O) \
	list_append_sized((L),&(O),sizeof((O)))

typedef struct {
	int val;
} Test;

int list_test(void) {
  List list = list_init();
  printf("\nCreated List\t:\t%p\n",&list);
  List *l = &list;
  printf("\nAppended four items\n");
 
  /* char* str = "first on heap!"; */        
  /* char* owned = malloc(strlen(str)+1); */
  /* strcpy(owned,str); */

  list_append(l, "First"); // owned
  char* slice = "Second";
  list_append(l, slice);
  list_append(l, "Third");
  list_append(l, "Four");
 

  printf("\nList: \n\t");
  list_print(l,"%s",char*);


  printf("\n\nSecond element: %s\n",(char*) list_get(l, 1));


  printf("\nPoping first element");
  list_pop_first(l);


  printf("\nList: \n\t");
  list_print(l,"%s",char*);

  printf("Poping last element 1 times");
  list_pop_last(l);
  
  list_insert_after(l,0,"insertion");

  printf("\nList: \n\t");
  list_print(l,"%s",char*);

  list_destroy(l);
  /* free(owned); */

  List struct_list = list_init();
  Test test = {2};
  list_append(&struct_list, &test );

  void* value = list_get(&struct_list,0);
  printf("DATA IN STRUCT: %d\n",
		  ((Test*)value)->val );

  list_destroy(&struct_list);
  return 0;
}




void test_args(int argc, char** argv) {
	Args a = args_init(argc,argv);

	char* value = arg_find_value(a,"-m");
	int help = arg_find(a,"--help");

	if (help)
	{
		printf("called for help!\n");
	}

	if (!value) 
		printf("not found flag or argument\n");
	else 
		printf("found! values is: `%s`\n",value);

	printf("arg_get_path: '%s'\n",arg_get_path(a));
	
}

Tree* tree_init(size_t typesize, size_t child_count) {
	void* data_ptr = calloc(1,typesize);
	assert(data_ptr && "failed to allocate data_ptr using calloc");

	Tree* t = calloc(1,sizeof(Tree));
	assert(t && "failed to allocate tree node using calloc");
	
	*t = (Tree) {
		.id 				= 0,
		.data 				= data_ptr,
		.parent 			= NULL,
		.children 			= NULL,
		.children_capacity 	= child_count,
	};

	return t;
}

void tree_join_right();
void tree_join_left ();

void tree_join(Tree* parent, Tree* children) {
	assert(parent->children_count < parent->children_capacity
			&& "Tree children is filled already");


	
}

void tree_free(Tree* tree) {
	if (!tree) 
		return;

	if (tree->children)
	{
		for (uint i = 0; i < tree->children_count; i++) {
			tree_free(tree->children[i]);
		}
	}
	free(tree);
	free(tree->children);
}


void test_tree(void) {
	Tree* t = tree_init(4,2);
	tree_free(t);
}

int main (int argc, char** argv) {

	test_tree();

	/* test_args(argc,argv); */
	// list_test();

	// HashMap m = mapinit(int, 10);

	// char* s = "EffectArmorFrailty";
	// printf("Hash of string \"%s\" -> %lu\n", s ,(djb2_hash(s) % 100));

}
