#if 0
#include "../src/link.h"
#include "../src/arena.h"
#include "../src/prelude.h"
#endif
#include "../hc.h"

typedef struct {
    const char* name;
    unsigned char age;
    ListHead;
} Person;


char* person_fmt(Person p) {
    static char temp[512];
    memset(temp,0,512);
    sprintf(temp, "{\"%s\", %i}", p.name, p.age);
    return temp;
} 


Person* arena_new_person(Arena* arena, char* name, unsigned int age) {
    Person* item = arena_alloc(arena, sizeof(Person));
    item->name = name;
    item->age = age;
    return item;
}


int main(void) {
    Person* list = 0;

    printf(" STACK ALLOCATED NODES \n");
    // ignore perentacies, they are to ignore stupid warning i care about
    Person juliet = {"juliet", 23, {}};
    Person romeo  = {"romeo",  25, {}};
    Person marco  = {"marco",  28, {}};

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
