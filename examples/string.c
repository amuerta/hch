#include "../src/string.h"
#include "../src/sb.h"

typedef hc_String String;

int main(void) {
    
    String s1,s2;
    
    s1 = str_make("\t  Testing trim and other  \n");
    printf("'%s'\n", str_temp_cstr(str_trim(s1)));
    
    printf("'%s'\n", str_temp_cstr(str_trim(s1)));

    s2 = str_dup(s1);
    printf("'%s'\n", str_temp_cstr(str_trim_right(s2)));


    str_reset(&s2);

    s1 = str_make("I am a string with pattern :0.");
    printf("'%s'\n", str_temp_cstr(s1));

    int pos = str_has_pattern(s1, str_make("pattern"));
    const char* cstr = str_temp_cstr(
        str_substr(s1, pos, str_make("pattern").count)
    );
    printf("'%s'\n" , cstr);


    String l,r;
    l = r = str_zero();
    l = str_make("SplitBySpace: "
            "\tOne "
            "\ttwo "
            "\tthree "
            "\tfour "
            "\t4.10 "
            "\t4. "
            "\tfive "
            "\t6 "
            "   hahaha"
    );

    for(int i = 0; !str_is_empty(l); i++) {
        r = str_split_by_chars(&l, " ");
        String trim = str_trim(r);
        if (str_is_integer(trim) || str_is_float(trim)) {
            printf("is number, ");
        }
        printf("%i '%s'\n", i, str_temp_cstr(r));
    }
}
