#include <hc_memory.h>
#include <hc_string.h>

typedef hc_String String;


int main(void) {
    
    String s1,s2;
    
    s1 = hc_string_make("\t  Testing trim and other  \n");
    printf("'%s'\n", hc_string_temp_cstr(hc_string_trim(s1)));
    
    printf("'%s'\n", hc_string_temp_cstr(hc_string_trim(s1)));

    s2 = hc_string_dup(s1);
    printf("'%s'\n", hc_string_temp_cstr(hc_string_trim_right(s2)));


    hc_string_reset(&s2);

    s1 = hc_string_make("I am a string with pattern :0.");
    printf("'%s'\n", hc_string_temp_cstr(s1));

    int pos = hc_string_seek_pattern(s1, hc_string_make("pattern"));
    const char* cstr = hc_string_temp_cstr(
        hc_string_substr(s1, pos, hc_string_make("pattern").count)
    );
    printf("'%s'\n" , cstr);


    String l,r;
    l = r = hc_string_zero();
    l = hc_string_make("SplitBySpace: "
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

    for(int i = 0; !hc_string_is_empty(l); i++) {
        r = hc_string_split_by_chars(&l, " ");
        String trim = hc_string_trim(r);
        if (hc_string_is_integer(trim) || hc_string_is_float(trim)) {
            printf("is number, ");
        }
        printf("%i '%s'\n", i, hc_string_temp_cstr(r));
    }
}
