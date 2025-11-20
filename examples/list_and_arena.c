#define INCLUDE_ARENA
#define HCH_ARENA_IMPLEMENTATION
#include "../hc.h"

typedef struct People {
    const char* name;
    unsigned char age;
    struct People *next, *tail, *prev;
} People;


char* person_fmt(People p) {
    static char temp[512];
    memset(temp,0,512);
    sprintf(temp, "{ \"%s\", %i }", p.name, p.age);
    return temp;
} 


People* arena_new_person(Arena* arena, mstr name, u8 age) {
    People* item = arena_alloc(arena, sizeof(People));
    item->name = name;
    item->age = age;
    return item;
}


int main(void) {
    People* list = 0;

    printf(" STACK ALLOCATED NODES \n");
    People juliet = { "juliet", 23, 0,0,0 };
    People romeo  = { "romeo",  25, 0,0,0 };
    People marco  = { "marco",  28, 0,0,0 };

    li_append(list, &juliet);
    li_append(list, &romeo);
    li_append(list, &marco);

    li_foreach(list, People, it, {
            printf("%s\n", person_fmt(*it));
    });

    // reset the stack list
    list = 0;

    printf(" ARENA ALLOCATED NODES \n");
    Arena people = {0};

    li_append(list, arena_new_person(&people,"victor", 32));
    li_append(list, arena_new_person(&people,"andrey", 41));
    li_append(list, arena_new_person(&people,"khor", 41));

    li_foreach(list, People, it, {
            printf("%s\n", person_fmt(*it));
    });


    list = 0;
    arena_clear(&people);

    printf(" ARENA RESET AND REUSED \n");
    li_append(list, arena_new_person(&people,"vicka", 33));
    li_append(list, arena_new_person(&people,"victora", 29));
    li_append(list, arena_new_person(&people,"alexa", 43));

    li_foreach(list, People, it, {
            printf("%s\n", person_fmt(*it));
    });


    arena_free(&people);
}
