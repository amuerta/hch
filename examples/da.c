#define DA_START_CAPACITY 2
// FOR TESTING
// #define DA_SIMPLER_IMPLEMENTATION
#include "../hc.h"

#ifdef DA_SIMPLER_IMPLEMENTATION
// OLDER EXAMPLE
typedef struct {
    const char* name;
    int age;
} Person;

typedef struct {
    Person* items;
    size_t count, capacity, typesize;
    // OR DA_HEADER(Person); // instead of all above
} People;

void person_add(People* ppl, const char* name, int age) {
    Person p = {name,age};
    da_append(ppl, p);
}


int main(void) {
    People ppl = {0};
    person_add(&ppl, "Jackson", 21);
    person_add(&ppl, "Amu", 20);
    person_add(&ppl, "Victor", 18);
    person_add(&ppl, "Gerald", 34);
    person_add(&ppl, "Garry", 28);

    da_foreach(ppl, i) {
        Person p = da_get(ppl)[i];
        printf("%lu %s-%i\n",i,p.name,p.age);
    }
    da_free(ppl);
}

#else
// NEWER EXAMPLE
typedef struct {
    int* items;
    DaHead;
} Numbers;

int main(void) {
    Numbers xs = {0};
    
    for(int i = 0; i < 128; i++) {
        da_append(&xs, i);
    }

    int one = -1, 
        two = -2 , 
        three = -3;
    da_append(&xs, one);
    da_append(&xs, two);
    da_append(&xs, three);

    da_foreach(xs, i) {
        printf("%i ", da_get(xs)[i]);
    }
    da_free(xs);
}
#endif

