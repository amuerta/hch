#define INCLUDE_STRING_BUILDER

#include <hc_memory.h>
#include <hc_string.h>

// Similar to `nob.h` string builder, I made mine for fun.



int main(void) {

    Allocator alloc = allocator_malloc();
    StringBuilder sb = {0};

    // you can use it as is to just build the string.
    stringb_append(alloc, &sb, "Hello", "World", 0 /*NULL is just skipped*/ , "!\t");
    stringb_appendf(alloc, &sb, "I write number %i!", 32);
    printf("String: '%s'\n", sb.items);
    stringb_clear(&sb);
   
    printf("\nYOU CAN USE IT AS PATH BUILDER TOO!!\n");
    sb.spacer = "/";
    stringb_append(alloc, &sb, "." , "config", "myapp" , "user", "config.ini");
    printf("Path: '%s'\n", sb.items);
    
    // you can do trailing path too:
    stringb_clear( &sb);
    stringb_append(alloc, &sb, "." , "config", "myapp" , "");
    printf("Path trailing: '%s'\n", sb.items);

    printf("\nOR JUST SPACE OUT YOUR PRINTING...\n");
    stringb_clear( &sb);
    sb.spacer = ", ";
    stringb_append(alloc, &sb, "1" , "2", "3", "4", "5");
    printf("Array: [ %s ]", sb.items);

    printf("\nCAN BE REVERSED IF NEEDED\n");
    stringb_reverse(&sb);
    printf("Array: [ %s ]\n", sb.items);

    free(sb.items);
}

