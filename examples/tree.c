#include "../src/link.h"

typedef struct Node {
    ImplementLink;
    int n;
} Node;

#define li_connect          hc_li_connect
#define li_append           hc_li_append
#define li_append_children  hc_li_append_children
#define li_foreach          hc_li_foreach
#define li_next             hc_li_next
#define li_children         hc_li_children
#define Link                hc_Link

Node make_n(int n) {
    Node it = {0};
    it.n = n;
    return it;
}

void traverse_tree(Link(Node) it, int depth) {
    if(!it) return;
    for(int i = 0; i < depth; i++) printf("  ");
    printf("%i\n", it->n);

    Node* children = hc_li_children(it);
    while(children) {
        traverse_tree(children, depth + 1);
        children = li_next(children);
    }
}

int main(void) {
    Link(Node) tree = 0;
    Link(Node) handle = 0;

    Node
        root    = make_n(1),
        child1  = make_n(2),
        child2  = make_n(3),
        child3  = make_n(4),
        child4  = make_n(5),
        child5  = make_n(6)
    ;

    li_append(tree, &root);

    li_append_children(tree, &child1);
    li_append_children(tree, &child2);
    li_append_children(tree, &child3);
    li_append_children(tree, &child4);

    handle = li_children(tree);
    li_append_children(handle, &child5);
    
    traverse_tree(tree, 0);
}
