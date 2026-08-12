
#define INCLUDE_ARGS
#include <hc_args.h>

int main(int argc, char** argv) {
    static ArgsRecord usage = {
        .usage_padding  = 2,
        .flag_padding   = 8,
    };
    static Args args = {0};
    args.record = &usage;
    args.items = argv;
    args.count = argc;

    usage.executable_path = argv[0];
    Args params = {0};
    bool help = false;

    if(hc_arg_flag_record(args, "help", "Prints usage.")) {
        printf("Requested help.\n");
        args.falltrough = true;
        help = true;
    }

    if(hc_arg_flag_record(args, "greet", "Prints greet message")) {
        printf("[Response] Hello! :)\n");
    }

    bool n = 0;
    bool n2 = 0;
    if(hc_arg_bool_record(&args, &n, "n", "prints true or false")) {
        printf("[N] %s\n", n ? "true" : "false");
    }

    if(hc_arg_bool_record(&args, &n2, "n2", "copy of n")) {
        printf("[N2] %s\n", n2 ? "true" : "false");
    }

    if(hc_arg_list_record(args, &params, "list", "int", "list of numbers")) {
        printf("list item count: %i\n", params.count);
        printf("list items: [ ");
        for(int i = 0; i < params.count; i++)
            printf("%s ", params.items[i]);
        printf("]\n");
    }

    

    if(*args.errors) {
        printf("errors: \n%s\n", args.errors);
        hc_argsrecord_print(usage);
    } else if(help) hc_argsrecord_print(usage);
    
}
