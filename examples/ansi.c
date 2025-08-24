//TODO:
// rename it to something else, like format.c or something
// add support for windows 7,8,10 before update 1704 where ansi support was introduced 
// into the console

// make 2 different api's for getting a color, ansi and old windows SetAttribute thing

// Way of doing temporary allocations that are ok? 
// as long as you do all operations and then .reset()

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static char ansi_fmt_temp_buffer[1024];
static int  ansi_fmt_temp_ptr;

static char* ansi_rgb(unsigned char r, unsigned char g, unsigned char b) {
    snprintf(ansi_fmt_temp_buffer + ansi_fmt_temp_ptr, 1024, "\e[38;2;%i;%i;%im", r, g, b);
    ansi_fmt_temp_ptr += 32;
    return ansi_fmt_temp_buffer + ansi_fmt_temp_ptr - 32;
}

static char* ansi_reset() {
    memset(ansi_fmt_temp_buffer, 0, 1024);
    ansi_fmt_temp_ptr = 0;
    return "\e[0m";
}

// Namespaces in C :o
static const struct {
    char* (*rgb) (unsigned char, unsigned char, unsigned char);    
    char* (*reset) (void);
    struct {
        const char*   red         ;
        const char*   green       ;
        const char*   blue        ;
        const char*   yellow      ;
        const char*   light_red   ;
        const char*   light_green ;
        const char*   light_blue  ;
        const char*   magneta     ;
        const char*   pink        ;
        const char*   cyan        ;
    } colors;
} ANSI = {
    .rgb   = ansi_rgb,
    .reset = ansi_reset,
    .colors = {
        .red         = "\e[38;2;255;0;0m",
        .green       = "\e[38;2;0;255;0m",
        .blue        = "\e[38;2;0;0;255m",
        .yellow      = "\e[38;2;255;255;0m",
        .light_red   = "\e[38;2;255;128;0m",
        .light_green = "\e[38;2;128;255;0m",
        .light_blue  = "\e[38;2;0;128;255m",
        .magneta     = "\e[38;2;255;0;255m",
        .pink        = "\e[38;2;255;0;127m",
        .cyan        = "\e[38;2;0;255;255m",
    }
};


int main(void) {
    static char fmt[256];
    sprintf(fmt, "%s%s%s%s%s" , ANSI.colors.red, "Hello", ANSI.rgb(30,100,30), "World",  ANSI.reset());
    printf("%s", fmt);

    return 0;
}
