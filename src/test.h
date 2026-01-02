#ifndef __HCH_TESTING_H
#define __HCH_TESTING_H

#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>


#define HC_TEST_FMT_SIZE 256
#define hc_test_expect(COND, OR) if (!(COND)) return (OR)

#ifndef hc_ignore
#define hc_ignore(...)      (void) (__VA_ARGS__)
#endif
#define hc_test_arrlen(A)   (sizeof(A)/sizeof(*A))
#define hc_test_wrap(...)   ((hc_Test[]) __VA_ARGS__)
#define hc_testlist(...)    { .items = hc_test_wrap(__VA_ARGS__), .count = hc_test_arrlen(hc_test_wrap(__VA_ARGS__)) }

typedef struct hc_Test hc_Test;
typedef struct {unsigned char r,g,b;} hc_TestColor;
typedef struct {
    bool    init;
    int     argc;
    void*   argv;
    void*   env;
} hc_TestArgs;
typedef int (*hc_TestFn) (hc_Test*, hc_TestArgs);

typedef struct hc_Test {
    char        name[HC_TEST_FMT_SIZE];
    char        comment[HC_TEST_FMT_SIZE];
    hc_TestFn   fn;
    hc_TestArgs args;
    int         result;
} hc_Test;
typedef struct {
    hc_Test* items;
    int      count;
} hc_Tests;

typedef struct {
    int style;
    hc_TestColor 
        ok, warn, error,
        highlight
            ;
} hc_TestStyle;


typedef struct {
    hc_Tests    tests;
    FILE*       out;
    hc_TestStyle  color;
} hc_Tester;

enum {
    HC_TEST_COLORSTYLE_NONE,
    HC_TEST_COLORSTYLE_ANSI,
};

enum {
    HC_TEST_OK,
    HC_TEST_SEMIOK,
    HC_TEST_FAIL,
    HC_TEST_OTHER,
}; 

static hc_Test EMPTY_TEST = {0};

hc_TestStyle hc_test_style_ansi(void) {
    hc_TestStyle s = {
        .style = HC_TEST_COLORSTYLE_ANSI,
        .highlight = {50,200,255},
        .ok =     {0,200,0},
        .warn =     {200,200,0},
        .error = {200,50,0},
    };
    return s;
}

const char* hc_test_ansi_rgb(unsigned char r, unsigned char g, unsigned char b) {
    static char temp[HC_TEST_FMT_SIZE];
    memset(temp, 0, sizeof(temp));
    snprintf(temp, sizeof(temp), "\e[38;2;%i;%i;%im", r, g, b);
    return temp;
}

const char* hc_test_ansi_reset(void) {
    return "\e[0m";
}

void hc_test_begin_color(hc_Tester t, hc_TestColor c) {
    if(!isatty(fileno(t.out))) return;
    switch (t.color.style) {
        case HC_TEST_COLORSTYLE_ANSI:
            fprintf(t.out, hc_test_ansi_rgb(c.r,c.g,c.b));
            break;
        default: break;
    }
}

void hc_test_end_color(hc_Tester t) {
    if(!isatty(fileno(t.out))) return;
    switch (t.color.style) {
        case HC_TEST_COLORSTYLE_ANSI:
            fprintf(t.out, hc_test_ansi_reset());
            break;
        default: break;
    }
}

void hc_test_comment(hc_Test* test, const char* comment) {
    int len = strlen(comment);
    memcpy(test->name, comment, len >= HC_TEST_FMT_SIZE ? HC_TEST_FMT_SIZE : len);
}

void hc_test_run(hc_Tester* t) {
    for(int i = 0; 
            i < t->tests.count || 
            memcmp(&t->tests.items[i], &EMPTY_TEST, sizeof(EMPTY_TEST)) == 0; 
            i++) 
    {
        hc_Test* test = t->tests.items + i;
        test->result = test->fn(test, test->args);
        
        hc_test_begin_color(*t, t->color.highlight);
        fprintf(t->out, "#%i\t", i);
        hc_test_end_color(*t);

        fprintf(t->out, "'%s'\t: ",  test->name);
        fprintf(t->out, "[");
        
        switch (test->result) {

            case HC_TEST_OK:
                hc_test_begin_color(*t, t->color.ok);
                fprintf(t->out, "OK");
                hc_test_end_color(*t);
                break;

            case HC_TEST_SEMIOK:
                hc_test_begin_color(*t, t->color.warn);
                fprintf(t->out, "WARN");
                hc_test_end_color(*t);
                break;

            case HC_TEST_FAIL:
                hc_test_begin_color(*t, t->color.error);
                fprintf(t->out, "FAIL");
                hc_test_end_color(*t);
                break;
            
            case HC_TEST_OTHER:
                hc_test_begin_color(*t, t->color.highlight);
                fprintf(t->out, "OTHR");
                hc_test_end_color(*t);
                break;
        }

        fprintf(t->out, "]");

        if(strlen(test->comment))
            fprintf(t->out, "\n\t| info: \"%s\"",  test->comment);

        fprintf(t->out, "\n");
    }
}

#define hc_test_make(fn_name, ...) hc_test_make_fn(#fn_name, fn_name, (hc_TestArgs) __VA_ARGS__)
hc_Test hc_test_make_fn(const char* name, hc_TestFn fn, hc_TestArgs args) {
    hc_Test t = {0};
    int len = strlen(name);
    memcpy(t.name, name, len >= HC_TEST_FMT_SIZE ? HC_TEST_FMT_SIZE : len);
    t.fn = fn;
    t.args = args;
    return t;
} 

#endif//__HCH_TESTING_H
