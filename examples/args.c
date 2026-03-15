
#define INCLUDE_ARGS
#include "../src/args.h"

int main(int argc, char** argv) {
    ArgsSlice args = {argv, argc};
    ArgsSlice params = {0};
    
    if(arg_flag(args, "greet")) {
        printf("[Response] Hello! :)\n");
    }

    bool n = 0;
    if(arg_bool(args, "n", &n)) {
        printf("[N] %s", n ? "true" : "false");
    }
    if(*args_error) printf("\nerror: %s", args_error);

    if(arg_list(args, "list", &params)) {
        printf("list item count: %i\n", params.count);
        printf("list items: [ ");
        for(int i = 0; i < params.count; i++)
            printf("%s ", params.items[i]);
        printf("]\n");
    }
}
