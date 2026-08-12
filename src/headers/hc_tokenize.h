/*********************************|Tokenize!|**********************************/
#ifndef __HC_TOKENIZE_H
#define __HC_TOKENIZE_H
/*
    # Tokenize - tokenizer in single function call.

        The idea was born when i was watching @Tsoding and considering 
        process of tokenization in my projects. Tokenizer or Lexer with a
        "context" struct seems too bulky for when you just wanna split off
        pieces of strings with kind tags (Tokens). But then it hit me!, 
        why even bother with tokenizer context struct when you can have 
        messy, ugly - yet compact, simple(relatively), single function 
        that only chops of pieces of source string and give you back tokens 
            _which are either_:
            - Numbers: integers, floats, (maybe in future hex, binary);
            - Words: keywords, identifiers (like_this_valid_variable_name);
            - Symbols: anything that doesn't fall into Words or Number but is a 
                character;
            - Custom: custom user defined token strings that either represent 
                    single character, sequence of characters or keywords, 
                    and are defined by TokenEntry table, 
                    and when parsed have highest priority;
        And then within your program you can easily manage recovery points when doing 
        parsing with source string pointers, by yourself count number of rows, columns, 
        etc.

EXAMPLE:
    ```c
        // These are from my Handy C Headers: https://github.com/amuerta/hch
        #include "../src/string.h"
        #include "../src/prelude.h"
        #include "../src/tokenize.h"

        #define String hc_String
        #define Token hc_Token
        #define TokenEntry hc_TokenEntry
        #define arrlen hc_arrlen


        int main(void) {
            hc_String src = str_make("int main() { printf(\"Hello\\\" %s!\", world);  }");
            hc_Token token;

            hc_TokenEntry tokens[] = {
                hc_token_entry("%s"),
            };
            while(token = str_chop_token_or_string(&src, tokens, 1, "\"","\""), 
                    hc_token_is_valid(token)) 
                printf("\t token: '%s',\t kind: %u\n", hc_token_to_cstring(token), token.kind);
            
        }
    ```

    NOTE: This "library" expects a length-based String implementation, by default it uses
    my implementation of it, but feel free to redefined it as anything that fits you. 

    KEEP IN MIND THAT STRUCTURE OF THE STRING IN MEMORY HAS TO BE 
    `struct { size_t, void* }` such that `sizeof(String) == 16`
 */
#ifndef __HC_STRING_H
    typedef struct {
        size_t      count;
        const char* items;
    } hc_String;
#endif

/*Change used string type here*/
typedef hc_String hc_token_String;


/*******************|Includes|*******************/
#include <stdbool.h>
#include <string.h>
#include <assert.h>

/**********|Helper stuff (ingnore it)|***********/
#define hc_token__max(A,B) (((A) > (B)) ? (A) : (B))
#define hc_token__min(A,B) (((A) < (B)) ? (A) : (B))


typedef enum {
   HC_TOKEN_ERROR_TOKENIZING,
   HC_TOKEN_INT,
   HC_TOKEN_FLT,
   HC_TOKEN_SYM,
   HC_TOKEN_WORD,
   HC_TOKEN_STRING, /*we have string as token kind but library doesn't produce those*/
   HC_TOKEN_CUSTOM,
   HC_TOKEN_END,
} hc_TokenKind;

typedef struct {
    hc_token_String      slice;
    hc_TokenKind   kind;
    union {
        int         integer;
        float       floating;
        char        symbol;
    } value;
} hc_Token;


typedef struct {
    int         custom_token_id; /*You can avoid string comparisons by assigning id*/
    hc_token_String  match;
} hc_TokenEntry;

bool hc_token_is_valid(hc_Token t);

bool hc_token_match_string(hc_Token t, hc_token_String slice);
bool hc_token_match_cstring(hc_Token t, const char *str);

/*TODO: swap table and table size, annotated count as token_table_count for clarity!*/
hc_Token hc_string_chop_token(hc_token_String* src, hc_TokenEntry* token_table, size_t count);
hc_Token hc_string_chop_token_opt(hc_token_String* src, hc_TokenEntry* token_table, size_t count, int flags);
/*                                                                                     ^^^^^^^^^
 * NOTE: I probably might add flags for emitting tokens when new_line is encountered or giving you
 * tokens of spaces and tabs, but for now i don't really care about that functionality, so these
 * flags literally do nothing. HOWEVER, they will be utilized in the future.
 */

hc_Token hc_string_chop_token_or_string(hc_token_String *src, 
        hc_TokenEntry *table, size_t count, 
        const char* opening, const char* closing);


/*                                                                  */
/*                         |IMPLEMENTATION|                         */
/*                                                                  */
#ifndef HC_TOKENIZE_HEADER_ONLY
/*************************|Helper functions|*************************/
hc_Token hc_string_chop_token(hc_token_String* src, hc_TokenEntry* token_table, size_t count) {
    return hc_string_chop_token_opt(src, token_table, count, 0);
}

bool hc_token_is_valid(hc_Token t) {
    return t.kind != HC_TOKEN_ERROR_TOKENIZING && t.kind != HC_TOKEN_END;
}

bool hc_token__is_space(char c) {
    return c == ' ' || c == '\n' || c == '\t';
}

bool hc_token__is_letter(char c) {
    return 0
        || ('a' <= c && c <= 'z') 
        || ('A' <= c && c <= 'Z');
}

bool hc_token__is_digit(char c) {
    return ('0' <= c && c <= '9'); 
}

hc_TokenEntry hc_token_entry(const char* string) {
    hc_TokenEntry entry = {0};
    entry.match = hc_string_make(string);
    return entry;
}

hc_TokenEntry hc_token_entry_with_id(const char* string, int id) {
    hc_TokenEntry entry   = {0};
    entry.match    = hc_string_make(string);
    entry.custom_token_id = id;
    return entry;
}


/****************************|Core logic|****************************/
/*
    Function parses int without modifying src string,
    until it finishes with correct result, then the changes
    are applied to `src` string and parsed token string is 
    put into `out->slice`

    other `hc_token__parse_*` functions are like this one,
    but each does parsing of their token kind.
 */
bool hc_token__parse_number(hc_Token* out, hc_token_String* src) {
    hc_token_String left = *src;
    hc_Token result = {0};
    bool parsing     = true;
    bool seen_period = false;
    size_t i = 0;

    if(!left.items || !left.count) return false;

    result.slice.items = left.items;
    for(i = 0; i < left.count && parsing; i++) {
        char c = *(left.items + i);
        if(c == '.') seen_period = true;
        /*not digit and (isnt a period or seen period) */
        else if(!hc_token__is_digit(c) && (c != '.' || seen_period)) 
            parsing = false, i--;
    }

    if(!i) return false; /*We didn't even begin looking for int 
                           in correct place. */


    /* If we are here - we have succesfully read a float or int*/
    char temp[128] = {0};/*I doubt youll need numbers bigger than 127 digits...*/
    memcpy(temp, result.slice.items, hc_token__min(i, sizeof(temp)));

    result.kind = seen_period ? HC_TOKEN_FLT : HC_TOKEN_INT;
    if(seen_period) 
        result.value.floating = atof(temp);
    else result.value.integer = atoi(temp);

    result.slice.count = i;
    src->items += i;
    src->count -= i;
    *out = result;
    return true;
}

bool hc_token__parse_word(hc_Token* out, hc_token_String* src) {
    hc_token_String left = *src;
    hc_Token result = {0};
    size_t i = 0; 
    bool parsing = true;

    if(!left.items || !left.count) return false;
    result.slice.items = left.items;
    
    for(i = 0; i < left.count && parsing; i++) {
        char c = *(left.items+i);
        bool filter = 0
            || (hc_token__is_letter(c) || c == '_') 
            || (i && hc_token__is_digit(c));
        if(!filter) /*for anything that is not valid word, stop reading it*/ 
            parsing = false,
            i--; /*rewind from invalid character*/
    }
    if(!i) return false;
    
    /*if were here, then word was sucessfully parsed*/
    result.kind = HC_TOKEN_WORD;
    result.slice.count = i;
    src->items += i;
    src->count -= i;

    *out = result;
    return true;
}

bool hc_token__parse_symbol(hc_Token* out, hc_token_String* src) {
    hc_token_String left = *src;
    hc_Token result = {0};

    if(!left.items || !left.count) return false;
    char c = *(left.items);
    result.slice.items = left.items;
    result.slice.count = 1;

    bool filter = 0 
        || hc_token__is_space(c)
        || hc_token__is_letter(c) 
        || hc_token__is_digit(c)    
        || ( c == '_' );
    if(filter) return false;

    src->items++;
    src->count--;
    result.kind = HC_TOKEN_SYM;
    result.value.symbol = c;
    *out = result;
    return true;
}



bool hc_token__parse_custom(hc_Token* out, hc_token_String* src, hc_TokenEntry* items, size_t items_count) {
    hc_token_String left = *src;
    hc_Token result = {0};
    bool found = false;
    hc_token_String match_token = {0};

    if(!left.items || !left.count)  return false;
    if(!items || !items_count)      return false;

    for(size_t e = 0; e < items_count && !found; e++) {
        match_token = items[e].match;
        if(!match_token.items || !match_token.count) continue;

        size_t remaining_count = left.count;
        /*today i learned C will always Short Circuit ||, && operators*/
        bool filter = remaining_count >= match_token.count 
            && !strncmp(match_token.items, left.items, match_token.count);
        if(filter) found = true;
    }

    if(!found) return false;

    result.kind        = HC_TOKEN_CUSTOM;
    result.slice.items = left.items;
    result.slice.count = match_token.count;

    src->count -= match_token.count;
    src->items += match_token.count;

    *out = result;
    return true;
}



hc_Token hc_string_chop_token_opt(hc_token_String* src, hc_TokenEntry* token_table, size_t count, int flags) {
    hc_Token    end     = {.kind = HC_TOKEN_END},
                result  = end;
    hc_token_String   cpy     = *src;/*avoid using the original variable whem mutating it*/
    
    (void) flags;
    assert(src->items);
    if(!src->count) return end;
    
    /*skip spaces*/
    while(hc_token__is_space(*(cpy.items))) 
        cpy.items++, cpy.count--;

    /*match tokens and parse them as you match them*/
    /*NOTE: src is NOT mutated on failure.*/
    if(hc_token__parse_custom(&result, &cpy,     token_table, count))    
        return (*src) = cpy, result;
    if(hc_token__parse_number(&result, &cpy))    return (*src) = cpy, result;
    if(hc_token__parse_word  (&result, &cpy))    return (*src) = cpy, result;
    if(hc_token__parse_symbol(&result, &cpy))    return (*src) = cpy, result;
    return result;
}

const char* hc_token_to_cstring(hc_Token t) {
    static char temp[512];
    memset(temp, 0, sizeof(temp));
    const size_t OF_COURSE_IT_ALSO_COUNTS_NULL_TERMINATOR = 1;
    /*TODO: count precision for floats too?*/
    size_t n = t.slice.count 
        + OF_COURSE_IT_ALSO_COUNTS_NULL_TERMINATOR;
    n = hc_token__min(n, sizeof(temp));
    switch(t.kind) {
        case HC_TOKEN_INT:      snprintf(temp, n, "%i", t.value.integer);  break;
        case HC_TOKEN_FLT:      snprintf(temp, n, "%f", t.value.floating); break;
        case HC_TOKEN_SYM:      n = 2, /*1 char + 1 NULL terminator*/
                                snprintf(temp, n, "%c", t.value.symbol);   break;
        case HC_TOKEN_STRING:   
        case HC_TOKEN_CUSTOM:   
        case HC_TOKEN_WORD:     snprintf(temp, n, "%.*s", (int)t.slice.count, t.slice.items);     break;
        case HC_TOKEN_END:              strcpy(temp, "END"); break;
        case HC_TOKEN_ERROR_TOKENIZING: strcpy(temp, "ERROR"); break;
    }
    return temp;
}

bool hc_token_match_string(hc_Token t, hc_token_String slice) {
    assert(slice.items);
    assert(t.kind);
    
    if(!slice.count)
        return false;
    else if (slice.count == 1)
        return (t.kind      == HC_TOKEN_SYM) 
            && (t.value.symbol == (*slice.items));
    else 
        return (t.slice.count == slice.count)
            && !strncmp(t.slice.items, slice.items, slice.count);
}

bool hc_token_match_cstring(hc_Token t, const char *str) {
    hc_String slice = {0};
    assert(str);
    slice.items = str; 
    slice.count = strlen(str);
    return hc_token_match_string(t, slice);
}

/* This is pretty string parsing function as 
 * the escaping is really primitive, but it's fine
 * as this function exists for quick and simple 
 * parsing of primitive strings and demostration
 * of how you can implement similiar functionality 
 * yourself once it's needed.                   */
hc_Token hc_string_chop_token_or_string(hc_token_String *src, 
        hc_TokenEntry *table, size_t count, 
        const char* opening, const char* closing) 
{
    hc_Token t, before = {0}; 
    hc_Token parsed_string;

    assert(src);
    assert(opening && closing);
    
    t = hc_string_chop_token(src, table, count);
    if(!hc_token_is_valid(t)) return t;

    assert(t.slice.items);
    parsed_string.slice.items = t.slice.items + strlen(opening);
    parsed_string.kind = HC_TOKEN_STRING;
    
    if(hc_token_match_cstring(t, opening)) {
        before = t;
        while(t = hc_string_chop_token(src, table, count), hc_token_is_valid(t)) {
            if(    !hc_token_match_cstring(before, "\\") 
                &&  hc_token_match_cstring(t, closing)) 
                goto end;       
            before = t;
        }

        end:
            parsed_string.slice.count = 
                t.slice.items - parsed_string.slice.items;
            return parsed_string; 
    }
    else return t;
}
#endif/*HC_TOKENIZE_HEADER_ONLY*/

#endif/*__HC_TOKENIZE_H*/

/****************************|CHANGELOG|*****************************/
/*
    Versions:
        v1.0 - Made initial library functionality with:
            hc_token_is_valid,
            hc_token_match_string,
            hc_token_match_cstring,
            string_chop_token,
            string_chop_token_opt - functions.
            NO LICENCE PROVIDED, I don't care.
*/
