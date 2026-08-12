#include "../src/link.h"
#include "../src/arena.h"
#include "../src/prelude.h"

#define li_append hc_li_append
#define li_foreach hc_li_foreach
#define List hc_Link

typedef struct Person {
    ImplementLink;
 
    const char* name;
    unsigned char age;
} Person;

char* person_fmt(Person p) {
    static char temp[512];
    memset(temp,0,512);
    sprintf(temp, "{\"%s\", %i}", p.name, p.age);
    return temp;
} 

List(Person) arena_new_person(Arena* arena, char* name, unsigned int age) {
    List(Person) item = arena_alloc(arena, sizeof(Person));
    item->name = name;
    item->age = age;
    return item;
}


Person person(const char* name, int age) {
    Person p = {0};
    p.name = name;
    p.age = age;
    return p;
}


int main(void) {
    List(Person) list = 0;

    printf(" STACK ALLOCATED NODES \n");
    Person juliet = person("juliet", 23);
    Person romeo  = person("romeo",  25);
    Person marco  = person("marco",  28);

    li_append(list, &juliet);
    li_append(list, &romeo);
    li_append(list, &marco);

    li_foreach(list, Person*, it) {
        printf("%s\n", person_fmt(*it));
    }

    // reset the stack list
    list = 0;

    printf(" ARENA ALLOCATED NODES \n");
    Arena people = {0};

    li_append(list, arena_new_person(&people,"victor", 32));
    li_append(list, arena_new_person(&people,"andrey", 41));
    li_append(list, arena_new_person(&people,"khor", 41));


    li_foreach(list, Person*, it) {
        printf("%s\n", person_fmt(*it));
    }

    list = 0;
    arena_clear(&people);

    printf(" ARENA RESET AND REUSED \n");
    li_append(list, arena_new_person(&people,"vicka", 33));
    li_append(list, arena_new_person(&people,"victora", 29));
    li_append(list, arena_new_person(&people,"alexa", 43));

    li_foreach(list, Person*, it) {
        printf("%s\n", person_fmt(*it));
    }

    arena_free(&people);
}
