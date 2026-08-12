#define NOB_IMPLEMENTATION
#define NOB_UNSTRIP_PREFIX
#include "lib/nob.h"
#include "src/headers/hc_args.h"

typedef struct {
    bool c89_or_c99;
    bool compile_w_mingw;
    bool address_sanitizer;
    bool debug_info;
    bool cpp_compatibility_test;
    bool run;
} BuildInfo; 

/*NOTE(IMPORTANT): THIS WILL UNCONTROLLABLY LEAK.
 * It is okay beacuse this program is not intended to run continiously.
 * But be aware of that!
 */
int main(int argc, char** argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);
    
    static ArgsRecord usage = {
        .usage_padding  = 2,
        .flag_padding   = 8,
    };
    static Args args = {0};

    Nob_Cmd cmd                   = {0};
    BuildInfo info                = {0};
    Nob_File_Paths files          = {0};
    Nob_File_Paths examples_files = {0};
    Nob_String_Builder pathb      = {0};
    Nob_String_View path          = {0};

    bool did_something = false;
    bool seeking_help = false;
#define help() if(true) {\
    args.falltrough = true;\
    seeking_help = true;\
    nob_log(NOB_INFO, "Requested help.");\
}

    args.record = &usage;
    args.items = argv;
    args.count = argc;
    usage.executable_path = argv[0];

    if(arg_flag_record(args, "help", "Prints script usage information.")) 
        help()

    if(arg_flag_record(args, "mingw", "USED WITH `build`. Compile as PE executable (Window) using MINGW compiler.")) 
        info.compile_w_mingw = true;

    if(arg_flag_record(args, "nerd", "USED WITH `build`. Apply all of the warnings (-Wall -Wextra).")) 
        info.compile_w_mingw = true;

    if(arg_flag_record(args, "debug", "USED WITH `build`. Enable ggdb debug symbols")) 
        info.debug_info = true;

    if(arg_flag_record(args, "sanitize", "USED WITH `build`. Enable memory sanitizer")) 
        info.address_sanitizer = true;

    if(arg_flag_record(args, "cppcompat", "USED WITH `build`. Check compatibility for C++.")) 
        info.cpp_compatibility_test = true;

    if(arg_flag_record(args, "run", "USED WITH `build`. Runs all examples."))
        info.run = true;
    
    if(arg_flag_record(args, "build", "Builds all examples with minimal compiler flags, unless told otherwise.")) {
        did_something = true;
        if(!nob_file_exists("./build.c")) {
            nob_log(NOB_ERROR, "USE THE SCRIPT FROM ROOT OF THE REPOSITOTY!!");
            goto end;
        }

        // out_dirput dir
        const char* out_dir = "./out";
        if(!nob_mkdir_if_not_exists(out_dir)) {
            nob_log(NOB_ERROR, "FAILED TO CREATE %s FOR EXAMPLES.", out_dir);
            goto end;
        }

        // Get list of examples, create a list of build commands.
        const char* examples = "./src/examples";
        const char* headers = "./src/headers";
        assert(nob_read_entire_dir(examples, &files));
        for(unsigned i = 0; i < files.count; i++) {
            pathb.count = 0;
            const char* file = files.items[i];
            if(!strcmp(file, ".") || !strcmp(file, "..")) 
                continue;
            nob_sb_appendf(&pathb, "%s/%s", examples, file);
            path = nob_sb_to_sv(pathb);
            const char* from = nob_temp_sv_to_cstr(path);

            if(nob_get_file_type(from) != NOB_FILE_REGULAR) {
                nob_log(NOB_WARNING, "SYMLINKS OR DIRECTORIES ARE IGNORED. name: (%s)", nob_temp_sv_to_cstr(path)); 
            } else {
                Nob_String_Builder argb = {0};
                cmd.count = 0; 
                {
                    //nob_cmd_append(&cmd, "echo");
                    nob_cmd_append(&cmd, "cc");

                    // output to
                    Nob_String_Builder out_pathb = {0};
                    nob_sb_appendf(&out_pathb, "%s/%s.example", out_dir, file);
                    const char* out  = nob_temp_sv_to_cstr(nob_sb_to_sv(out_pathb));
                    nob_cmd_append(&cmd, "-o", out);
 
                    if(info.run) {
                        nob_da_append(&examples_files, strdup(out)); // note: leak
                    }

                    // from where
                    nob_cmd_append(&cmd, from);

                    // Headers from examples are in include path. --include-directory=<PATH>
                    nob_sb_appendf(&argb, "--include-directory=%s", headers);
                    nob_cmd_append(&cmd, nob_temp_sv_to_cstr(nob_sb_to_sv(argb)));

                    // Link math.
                    nob_cmd_append(&cmd, "-lm");

                    if(info.debug_info) 
                        nob_cmd_append(&cmd, "-ggdb");

                    if(info.address_sanitizer) 
                        nob_cmd_append(&cmd, "-fsanitize=address");

                    //nob_log(NOB_INFO, "examples/%s compiles to %s", file, out);
                    
                    if(!nob_cmd_run_sync(cmd)) {
                        nob_log(NOB_ERROR, "Building %s failed! Terminating!", from);
                        goto end;
                    }
                }
            }
        }


        // run examples if asked.
        if(info.run) for(unsigned i = 0; i < examples_files.count; i++) {
            const char* example = examples_files.items[i];
            cmd.count = 0;
            nob_cmd_append(&cmd, example);
            if(!nob_cmd_run_sync(cmd)) {
                nob_log(NOB_ERROR, "Running example %s failed! Terminating!", example);
                goto end;
            }
        }
    }


    const char* test_subject = 0;
    if(arg_string_record(&args, &test_subject, "test", "Builds and runs specific example.")) {
        did_something = true;
    }


end:
    if(*args.errors || seeking_help) {
        argsrecord_print(usage);
        if(*args.errors) 
            printf("errors: \n%s\n", args.errors);
        return 0;
    }
    if(did_something) return 0;

    nob_log(NOB_INFO, "We did nothing. Maybe... check `--help`..?");
    return 0;
}
