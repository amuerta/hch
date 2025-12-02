#include "../src/string.h"

#define str_make(s) str_from_cstr(s, strlen(s))

int main(void) {
    
    String s1,s2,s3,*sl;
    
    s1 = str_make("\t  Testing trim and other  \n");
    printf("'%s'\n", str_temp_cstr(str_trim(s1)));
    
    str_reverse(&s1);
    printf("'%s'\n", str_temp_cstr(str_trim(s1)));

    s2 = str_dup(s1);
    str_reverse(&s2);
    printf("'%s'\n", str_temp_cstr(str_trim_right(s2)));


    //str_clear(&s1);
    str_clear(&s2);

    str_set(&s1, "I am a string with pattern :0.");
    printf("'%s'\n", str_temp_cstr(s1));


    str_free(&s1);
    str_free(&s2);
}
