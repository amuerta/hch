#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

// TODO: implement UTF-8 decoding/encoding functions 
// and make a verions of this that supports arbituary count of leaf's?

//
// ASCII-oriented implementation of tire
//

#define TIRE_MAX_CHILDREN 128
typedef uint32_t tire_ptr; 

typedef struct TrieNode {
    char                symbol;
    void*               user_data;
    uint8_t             children_count;
    struct TrieNode*    children[TIRE_MAX_CHILDREN];
} TrieNode;

#define PrefixTree Tire
typedef struct {
    void*     allocator; // pointer to custom struct
    TrieNode* (*alloc)(void*);
    TrieNode* root;
} Trie;

TrieNode* my_alloc(void* _ignored) {
    (void) _ignored;
    return calloc(1,sizeof(TrieNode));
}

void trie_node_add(Trie* trie, TrieNode* n, const char* word, char symbol) {
    assert(n);
    assert(word);
    char sym = *word;

    if(!sym) return;

    if(!n->children[sym]) {
        n->children[sym] = trie->alloc(0);
        n->children[sym]->symbol = sym;
    }

    trie_node_add(trie,n->children[*word], word+1,*word);
}


void trie_add(Trie* t, const char* word) {
    if(!t->root) t->root = t->alloc(0);
    t->root->symbol = 0;
    trie_node_add(t,t->root,word, *word);
}

void trie_print_node(TrieNode* n, int depth) {
    if (n->symbol)
        printf("%*s'%c'\n", depth, ">>", n->symbol);
    else
        printf("%*s'root'\n", depth, ">>");
    for( int i = 0, c = 0; 
         i < TIRE_MAX_CHILDREN || c < n->children_count; 
         i++) 
    {
        if(n->children[i]) trie_print_node(n->children[i],depth+1);
    }
}

void trie_print(Trie t) {
    trie_print_node(t.root, 0);
}

int main(void) {
    Trie t = {
        .alloc = my_alloc
    };
    trie_add(&t,"cat");
    trie_add(&t,"car");
    trie_add(&t,"cash");
    trie_add(&t,"carrot");
    trie_print(t);
}
