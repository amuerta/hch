#define INCLUDE_STRING_BUILDER

#include "../hc.h"

// similar to nob.h string builder, i made mine for fun.



int main(void) {
    StringBuilder sb = {0};

    // you can use it as is to just build the string.
    sb_append(&sb, "Hello", "World", 0 /*NULL is just skipped*/ , "!\t");
    sb_appendf(&sb, "I write number %i!", 32);
    printf("String: '%s'\n", sb.items);
    sb_clear(&sb);
   
    printf("\nYOU CAN USE IT AS PATH BUILDER TOO!!\n");
    sb.spacer = "/";
    sb_append(&sb, "." , "config", "myapp" , "user", "config.ini");
    printf("Path: '%s'\n", sb.items);
    
    // you can do trailing path too:
    sb_clear(&sb);
    sb_append(&sb, "." , "config", "myapp" , "");
    printf("Path trailing: '%s'\n", sb.items);

    printf("\nOR JUST SPACE OUT YOUR PRINTING...\n");
    sb_clear(&sb);
    sb.spacer = ", ";
    sb_append(&sb, "1" , "2", "3", "4", "5");
    printf("Array: [ %s ]", sb.items);


    free(sb.items);
}

