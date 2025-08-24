#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <stdbool.h>


// TODO:
//  + item size check for: insertion, list merge, join .
// 	+ bitmask config similar to array 
// 	+ destructor function for custom data
//
// [v]	list_from		(array, array_len) -> List
// [v] 	list_insert	(list,index) -> void
// [v]  list_dup	(list)		-> list
// [v]  list_pinch	(list,index)	-> void*
// [v]  list_inject	(list, index, list) -> void
// [v] 	list_split 	(list orig,index,return right, return left) -> void
// [v] 	list_join	 	(list l, list r) -> list 
// [ ] 	list_sublist 	(list l, index, count) -> list 
//	+	list_swap		(list l, index, index) -> void

// MAYBE_TODO:
//	+ list_slice	(list l, count, indexes for slicing ... ) -> Collection<list>
//	+ list_sort		(list l, *cmprer() 			) 	->	void
//	+ list_reverse  (list l)						->	void

// CROSS_HEADER_SUPPORT(MAYBE):
// 	+ list_from_array(Array) -> List

#define LIST_MALLOC malloc
#define LIST_FREE	free

#define LIST_IGNORE_RETURN (void)

typedef enum {
	// Like in other HC headers, forces
	// all functions that frees items 
	// using LIST_FREE
	// of the list to call user-defined
	// destroy(void*) function, that
	// can deallocate your complex custom 
	// structs, types, arrays, etc.
	LISTFLAG_USE_DESTRUCTOR 	= 1,


	// Make every function that creates
	// copy of the nodes, consume them.
	//
	// This means that list becomes invalidated
	// after use of the function that consumes it, 
	// and needs to be reinitilized.
	//
	// If consumed list given to any function. It will
	// assert "list consumed" error.
	//
	// Option interact with these functions:
	// 	+	list_inject	()
	// 	+ 	list_split 	()
	// 	+	list_join	()
	// 	+	list_sublist()
	LISTFLAG_CONSUME_NOT_CLONE 	= 2,

} ListFlags;

typedef int list_bitmask;

typedef void (*ListDestructor) (void*);

typedef struct ListNode {
	size_t				index;
	struct ListNode*	next;
	struct ListNode*	prev;
	void* 				data;
} ListNode;

typedef struct {
	bool			consumed;
	ListNode* 		head;
	ListNode* 		tail;
	list_bitmask 	flags;
	ListDestructor	destroy;
	size_t			item_sz;
	size_t 			length;
} List;



#define list_get(LIST,INDEX,TYPE) \
	*( (TYPE*) list_refer((LIST),(INDEX)) )

#define list_append(LIST,ITEM) \
	__list_append((LIST),(ITEM),sizeof(*(ITEM)))

#define list_insert(LIST,INDEX,ITEM) \
	__list_insert((LIST),(INDEX),(ITEM),sizeof(*(ITEM)))

// iterates over each item in list and gives out a
// user (ITER) void* ptr of the node data
#define list_foreach(LIST,ITER,...)  do {		\
		ListNode* node = (LIST).head;			\
		while(node) {							\
			ITER = node->data;					\
			if (1)	{__VA_ARGS__}				\
			node = node->next;					\
		}										\
	} while(0);									\

// acts exactly like list_foreach, but iterates
// over items in reverse order instead
#define list_eachfor(LIST,ITER,...)  do {		\
		ListNode* node = (LIST).tail;			\
		while(node) {							\
			ITER = node->data;					\
			if (1)	{__VA_ARGS__}				\
			node = node->prev;					\
		}										\
	} while(0);									\

#define list_init(LIST,TYPE)	\
	LIST_IGNORE_RETURN __list_init((LIST),sizeof(TYPE),NULL)

#define list_new(TYPE) \
	__list_init(NULL,sizeof(TYPE),NULL)


void __list_append(List* l, void* data, size_t sz);


List __list_init(List* l, size_t item_size,ListNode* head) {
	List new = {0};
	if (!l) 
		l = &new;

	memset(l,0,sizeof(*l));
	l->item_sz = item_size;

	if (head)
		l->head = head;
	return *l;
}

#define list_from(ARRAY,LENGTH) \
	__list_from((ARRAY),sizeof(ARRAY[0]),LENGTH)

List __list_from(void* array, size_t item_sz, size_t arr_length) {
	assert(array && "Array cannot be used when its NULL");
	assert(item_sz && "Type has to have valid size, got 0 - (void)");
	assert(arr_length && "cannot init with array_length = 0, use list_init(TYPE) instead");
	List l = __list_init(NULL,item_sz,NULL);
	for(size_t i = 0; i < arr_length; i++) {
		__list_append(&l, array+(i*item_sz), item_sz);
	}
	return l;
}

ListNode* __node_init(void* data, size_t sz) {
	ListNode* new = LIST_MALLOC(sizeof(*new));
	memset(new,0,sizeof(*new));

	new->data = LIST_MALLOC(sz);
	memcpy(new->data,data,sz);
	
	return new;
}

// node1      node2     0
//   next  ->  next   ->
//   prev  <-  prev
ListNode* __node_join(ListNode* l, ListNode* r) {
	l->next = r;
	r->prev = l;
	return r;
}

//		Left				Right
// -> [ next	x->x	 [ next ->
// <- ] prev	x<-x	 ] prev <-
// Disconnect right node from left,
// return left node as a by-product
ListNode* __node_disconnect(ListNode* l, ListNode* r) {
	l->next = 0;
	r->prev = 0;
	return l;
}

void __list_reindex(List* l) {
	ListNode* node = l->head;
	if(!node) return;
	size_t counter = 0;
	while(node) {
		node->index = counter++;
		node = node->next;
	}
	l->length = counter;
}


ListNode* __node_get(List* l, size_t i) {
	// TEMP: if i mess up, i'll know
	assert(l->length > i && "List indexing overflow, i >= list->length");

	ListNode* node;
	ListNode* yield = {0};

	// [x,x,x,x,x,x,x,x...] length = n
	//          ^
	//    i <  n/2   < i
	// ~~~~~~~~~   ~~~~~~~
	// O(n/2) !?
	if (i > l->length / 2) {
		node = l->tail;
		while(node) {
			if (node->index == i) 
				yield = node;
			node = node->prev;
		}
	} else {
		node = l->head;
		while(node) {
			if (node->index == i) 
				yield = node;
			node = node->next;
		}
	}
	return yield;
}

void	__node_free(ListNode* node,ListDestructor destroy_fn) {
	assert(node && "got null, expected node*");
	if (destroy_fn)
		destroy_fn(node->data);
	else
		LIST_FREE(node->data);
	LIST_FREE(node);
	node = NULL;
}


void __list_append(List* l, void* data, size_t sz) {
	assert(l && "List is not initilized");
	assert(data && "Data is NULL, expected valid pointer");
	assert(sz && "input type cannot be zero-sized, (void) type is forbidden");
	ListNode* new = __node_init(data,sz);
	new->index = l->length;
	// we have had a tail
	// join it to the tail and 
	// refresh l->tail ptr
	if (l->tail) {
		l->tail = __node_join(l->tail,new);
	}
	// we only had head node
	// create a l->tail
	else if (l->head) {	
		l->tail = __node_join(l->head,new);
	}

	// we append the first item 
	// into a list
	else {
		l->head = new;
	}
	l->length++;
}

void __list_insert(List* l, size_t index, void* data, size_t sz) {
	assert(l && "List is not initilized");
	assert(l->length > index && "List indexing overflow, i >= list->length");
	assert(data && "Data is NULL, expected valid pointer");
	assert(sz && "input type cannot be zero-sized, (void) type is forbidden");
	
	
	// we just append 
	if (index == l->length - 1) {
		__list_append(l,data,sz);
		return;
	} 

	// otherwise do some shenanigans
	ListNode* new = __node_init(data,sz);
	if (index == 0) {
		__node_join(new, l->head);
		l->head = new;
	}
	else {
		ListNode* target = __node_get(l,index);
		ListNode* prev =  __node_get(l,index)->prev;

		// [] [] []
		// [] xx [] []
		//     +->>
		__node_join(__node_join(prev, new), target);
	}

	l->length++;
	__list_reindex(l);
}

void __node_recursive_free(ListNode* node, ListDestructor destroy_fn) {
	if (!node)
		return;

	if (node->next) 
		__node_recursive_free(node->next, destroy_fn);
	
	if (destroy_fn)
		destroy_fn(node->data);
	else
		LIST_FREE(node->data);
	LIST_FREE(node);
}


void __list_free(List* l) {
	
	// TODO: figureout iterative way of freeing
	// so no "stackoverflow" can possibly happen
#if 0
	ListNode* node = l->tail;
	while(node && node->prev) {
		node = node->prev;
		__node_free(node->next);
	}
#endif
	if (l->flags & LISTFLAG_USE_DESTRUCTOR)
		assert(l->destroy && 
				"Expected to have provided destructor function with flag: LISTFLAG_USE_DESTRUCTOR");

	__node_recursive_free(l->head, l->destroy);
}

void __list_iter(List* l, void (*callback) (size_t i, void*)) {
	ListNode* node = l->head;
	if(!node)
		return;
	while(node) {
		callback(node->index,node->data);
		node = node->next;
	}
}

void __list_iter_reverse(List* l, void (*callback) (size_t i, void*)) {
	ListNode* node = l->tail;
	if(!node)
		return;
	while(node) {
		callback(node->index,node->data);
		node = node->prev;
	}
}

void callback(size_t i, void* data) {
	
	//if (!data)
	//	return;

	int* num = data;
	printf("item#%lu -> %i\n",i,*num);
}

List list_dup(List l) {
	List new = __list_init(NULL,l.item_sz, NULL);
	ListNode* node = l.head;
	if(node) while(node) {
		__list_append(&new,node->data,l.item_sz);
		node = node->next;
	}
	return new;
}

void list_clear(List* l) {				
	__list_free(l);	
	l->length = 0;						
} 

void list_reset(List* l) {				
	__list_free(l);	
	memset(l,0,sizeof(*(l)));		
} 


void* list_refer(List* l, size_t i) {
	return (__node_get(l,i))->data;
}


void list_pop(List* l) {
	if (l->flags & LISTFLAG_USE_DESTRUCTOR)
		assert(l->destroy && 
				"Expected to have provided destructor function with flag: LISTFLAG_USE_DESTRUCTOR");
	//temp
	assert(l->length && "Expect length > 0");

	if(l->length == 1) {
		__node_free(l->head,l->destroy);
		l->head = NULL;
		l->tail = NULL;
	} 
	else {
		ListNode* prev_tail = l->tail;
		l->tail = __node_disconnect(
				l->tail->prev,l->tail);
		__node_free(prev_tail,l->destroy);
	}
	l->length--;
}

void list_pop_at(List* l, size_t i) {
	assert(l->length > i && "List indexing overflow, i >= list->length");
	if (l->flags & LISTFLAG_USE_DESTRUCTOR)
		assert(l->destroy && 
				"Expected to have provided destructor function with flag: LISTFLAG_USE_DESTRUCTOR");
	
	if(i == l->length - 1) {
		list_pop(l);
	} else if (i == 0) {
		ListNode* new = l->head->next;
		__node_free(l->head,l->destroy);
		l->head = new;
		l->head->prev = NULL;
	} else {
		ListNode *n = __node_get(l,i);
		__node_join(n->prev,n->next);
		__node_free(n,l->destroy);
	}
	l->length--;
	__list_reindex(l);
}

void* list_pinch(List* l, size_t index) {
	void* ref = list_refer(l,index);
	void* new = LIST_MALLOC(l->item_sz);
	memset(new,0,l->item_sz); 
	// just to ensure that in any case
	// when doing a copy of the data,
	// no trailing garbage can corrupt 
	// a data cell.
	memcpy(new,ref,l->item_sz);
	list_pop_at(l,index);
	return new;
}

List __list_join(List* l, List *r, bool consume) {
	assert(l && r && "Both List has to be valid, (got NULL)");
	assert(l->length && r->length && "expected to have both lists ot be non empty");

	if (consume) {
		r->consumed = true;
		__node_join(l->tail,r->head);
		__list_reindex(l);
		return *l;
	}

	else {
		List lcopy = list_dup(*l);
		List rcopy = list_dup(*r);
		__node_join(lcopy.tail,rcopy.head);
		__list_reindex(&lcopy);
		return lcopy;
	}
}

List list_join(List* l, List* r) {
	// TODO: handle behaviour depending on 
	// set flags on both lists
	//
	//bool consume = (l->flags & LISTFLAG_CONSUME_NOT_CLONE);
	//return __list_join(l,r,consume);
	return __list_join(l,r,false);
}

void list_inject(List* l, size_t index, List* sublist) {
	assert(l && sublist && "expected to have both List's initlized");
	assert(l->length >= index && "List indexing overflow, i >= list->length");
	/* TODO:
	List subl = (l->flags & LISTFLAG_CONSUME_NOT_CLONE) ?
		*sublist : list_dup(*sublist);
	*/
	List subl = list_dup(*sublist);

	if (index == 0) {
		__node_join(subl.tail, l->head);
		l->head = subl.head;
		l->head->prev = NULL;
	}
	else if (index == l->length) {
		List new_l = __list_join(l,&subl,true);
		// list_join does clone on its own if
		// no consume is false.
		// we dont have to allocate and free here
		// since it's just waste of execution time.
		*l = new_l;
	}
	else {
		ListNode* target = __node_get(l,index);
		// [] [] [] []
		//   ^#1
		//
		// [] <-> { [] [] [] } <-> [] [] []
		__node_join(target->prev,subl.head);
		__node_join(subl.tail,target);
	}

	l->length = l->length + subl.length;
	__list_reindex(l);
}

bool list_is_empty(List l) {
	return ( (!l.head) && (!l.tail) ) || (l.length == 0);
}

void list_split(List* src, size_t index, List* lr, List* rr) {
	assert(list_is_empty(*rr) && list_is_empty(*lr) &&
			"Expected to have empty list for writing into");

	assert(index != 0 && index != src->length 
			&& "Pointless call of list_split");
	// TODO: maybe handle this situation?
	// I don't really know if it makes sense,
	// so its a "MAYBE, in future" todo comment
	// if i ever feel like i need it for some reason.
	// I doubt it will be. i may remove this
#if 0
	if (index == 0 || index == src->length - 1) {
		// do stuff for handling pointless split ? //
		memset(lr,0,sizeof(*lr));
		memset(rr,0,sizeof(*rr));
	}
#endif
	if (0){}

	else {
		/* TODO:
		   List src_dup = (src->flags & LISTFLAG_CONSUME_NOT_CLONE) ?
		 		*src : list_dup(*src);
		 */
		List src_dup = list_dup(*src);

		ListNode* target = __node_get(&src_dup,index);
		__node_disconnect(target->prev,target);
		
		ListNode* l = src_dup.head;
		ListNode* r = target;

		*lr = __list_init(NULL, src->item_sz, l);
		*rr = __list_init(NULL, src->item_sz, r);
		
		__list_reindex(lr);
		__list_reindex(rr);
	}
}

int main(void) {

	int n = 1;

	List l = list_new(int);
	
	//for(int i = 0; i < 100; i++ )
	list_append(&l,&n);
	list_append(&l,&n);n++;
	list_append(&l,&n);n++;
	list_append(&l,&n);n++;
	list_append(&l,&n);n++;
	list_append(&l,&n);n++;
	n = 100;
	printf("\t list l :\n");
	
	list_foreach(l,int* it,
			int *i = it;
			printf("%i\n",*i);
	);
#if 0
	printf("First element: %i\n", *((int*)l.head) );
	printf("Last  element: %i\n", *((int*)l.tail) );
#endif
	//__list_reindex(&l);
	
	int* item = list_refer(&l, 1);
	printf("refered to item#1: %i\n", *item);


	int itemv = list_get(&l, 1, int);
	printf("got dereferenced item#1: %i\n", itemv);

	List ldup = list_dup(l);

	int* pinched = list_pinch(&ldup,2);
	printf("pinched off item from index 2: %i\n", *pinched);
	free(pinched);
	
	list_pop(&ldup);
	list_insert(&ldup,1,&n);


	printf("\t edited copy of l - ldup:\n");

	list_foreach(ldup,int* it,
			int *i = it;
			printf("%i\n",*i);
	);

	printf("List 'ldup' injected to list 'l' at index 2\n");

	list_inject(&l,6,&ldup);

	list_foreach(l,int* it,
			int *i = it;
			printf("%i\n",*i);
	);

	printf("split lists back at index 2\n");
	List lr = {0}, rr = {0};
	list_split(&l, 10, &lr, &rr);


	printf("lr\n");
	list_foreach(lr,int* it,
			int *i = it;
			printf("%i\n",*i);
	);

	printf("rr\n");
	list_foreach(rr,int* it,
			int *i = it;
			printf("%i\n",*i);
	);

	printf("lists joined back\n");

	List joined = list_join(&lr,&rr);

	list_foreach(joined,int* it,
			int *i = it;
			printf("%i\n",*i);
	);

	int arr[] = {1,2,3,4,5};
	List from = list_from(arr,5);

	printf("created from array: ");
	list_foreach(from,int* it, 
			printf("{%i} ", *it);
	);

	list_clear(&from);

	list_clear(&joined);
	list_clear(&lr);
	list_clear(&rr);
	list_clear(&l);
	list_clear(&ldup);
}
