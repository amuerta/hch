#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

// 
// Amuerta's Markdown Parser
//

typedef struct {
    const char* data;
    size_t      length;
} amp_Slice;

typedef struct {
    int         type;
    int         properties;
    int         header_size;
    int         indentation;
    amp_Slice   text;
    amp_Slice   link;
} amp_Token;

typedef struct {
    amp_Slice   source;
    size_t      offset;
    int         indentation;
    int         current_type;
    int         state; // is_bold, is_crossed ... etc
} amp_Parser;

// type
enum {
    AMP_NULL,
    AMP_HEADER,
    AMP_PARAGRAPH,
    AMP_TEXT,
    AMP_BREAK,
    AMP_LINK,
    AMP_LIST,
    AMP_CODEBLOCK,
};

// properties
enum {
    AMP_IS_BOLD         = (1 << 0),
    AMP_IS_ITALIC       = (1 << 1),

    AMP_IS_CROSSED      = (1 << 2),
    AMP_IS_HIGHLIGHT    = (1 << 3),
    AMP_IS_UNDERLINED   = (1 << 4),

    AMP_IS_CODE1        = (1 << 5),
    AMP_IS_CODE2        = (1 << 6),
    
    AMP_IS_SEEN_CONTENT = (1 << 16),
};

enum {
    AMP_FLAG_SPLIT_BY_SPACE = (1 << 0),
};

amp_Slice amp_slice(const char* string) {
    amp_Slice s = {.data = string, .length = strlen(string)}; 
    return s;
}

bool amp_is_space(char c) {
    return c == '\t' || c == ' ' || c == '\v';
}

bool amp_is_word(char c) {
    return 
            ((c >= 'a') && ( c <= 'z' )) ||
            ((c >= 'A') && ( c <= 'Z' )) ||
            ((c >= '0') && ( c <= '9' )) 
            //|| c == ' '
    ;
}

bool amp_is_eol(char c) {
    return  c == '\n' || c == '\r';
}

bool amp_is_format(char c) {
    return 
        c == '*' || c == '_' ||
        c == '~' || c == '<' ||
        c == '`' || c == '=' 
    ;
}

amp_Token amp_get_header(amp_Parser* p) {
    amp_Token t = {0};
    const char* base = p->source.data + p->offset;
    // char current = *base;
    p->state |= AMP_IS_SEEN_CONTENT;
    
    while(p->offset < p->source.length && *(base + t.header_size) == '#') 
        t.header_size++;
    
    
    p->offset += t.header_size;
    t.text.data = base + t.header_size;
    t.type = AMP_HEADER;
    p->current_type = AMP_HEADER;
    return t;
}

amp_Token amp_get_text(amp_Parser* p) {
    amp_Token t = {0};
    const char* base = p->source.data + p->offset;
    const char* current = base;
    p->state |= AMP_IS_SEEN_CONTENT;

    while(p->offset < p->source.length &&
            !amp_is_eol(*current)      &&
            !amp_is_format(*current)   &&
            !amp_is_space(*current)) {
        t.text.length++;
        p->offset++;
        
        current = base + t.text.length;
    }

    t.indentation = p->indentation;
    
    if (!p->current_type)
        p->current_type = AMP_PARAGRAPH;
    t.type = AMP_TEXT;
    t.text.data = base;
    t.properties = p->state;
    return t;
}

char amp_next(amp_Parser* p, int n) {
     return p->offset + n < p->source.length ? 
         *(p->source.data + p->offset + n) : 0;
}

amp_Token amp_get_list(amp_Parser* p) {
    amp_Token t = {0};
    const char* base = p->source.data + p->offset;
    const char* current = base;
    p->state |= AMP_IS_SEEN_CONTENT;

    assert(*current == '-' && amp_is_space(amp_next(p, 1)));
    p->offset++;
    t.indentation = p->indentation;
    
    if (!p->current_type)
        p->current_type = AMP_LIST;
    t.type = AMP_LIST;
    t.properties = p->state;
    return t;
}



void amp_get_format(amp_Parser* p) {
    amp_Token t = {0};
    const char* base = p->source.data + p->offset;
    p->state |= AMP_IS_SEEN_CONTENT;

    while(amp_is_format(*base)) {
        switch(*base) {

            case '*':
                if (amp_next(p, 1) == '*') {
                    p->offset += 2;
                    p->state ^= AMP_IS_BOLD;
                } else {
                    p->offset += 1;
                    p->state ^= AMP_IS_ITALIC;
                }
            break;

            case '~':
                if (amp_next(p, 1) == '~') {
                    p->offset += 2;
                    p->state ^= AMP_IS_CROSSED;
                }
            break;

            case '=':
                if (amp_next(p, 1) == '=') {
                    p->offset += 2;
                    p->state ^= AMP_IS_HIGHLIGHT;
                }
            break;

            case '`':
                if (amp_next(p, 1) == '`') {
                    p->offset += 2;
                    p->state ^= AMP_IS_CODE1;
                } else {
                    p->offset += 1;
                    p->state ^= AMP_IS_CODE2;
                }
            break;

        }

        base = p->source.data + p->offset;
    }
}

amp_Token amp_get_break(amp_Parser* p) {
    amp_Token t = {0};
    const char* base = p->source.data + p->offset;
    size_t counter = 0;

    while(p->offset < p->source.length && *(base + counter) == '\n') 
        counter++;

    p->current_type = 0;
    p->state = 0;
    p->indentation = 0;
    p->offset += counter;
    t.type = AMP_BREAK;
    return t;
}

amp_Token amp_get_token(amp_Parser* p) {
amp_try_again:

    const char* data = p->source.data;
    const char* remains = data + p->offset;
    char  current = *remains;
   // char  next    = p->offset + 1 < p->source.length ? *(remains + 1) : 0;
    amp_Token t = {0};

    // END
    if (p->offset >= p->source.length) return t;

    if (amp_is_eol(current))            return amp_get_break(p);
    else if (amp_is_space(current)) {
        if (!(p->state & AMP_IS_SEEN_CONTENT)) {
            p->indentation += 1;
        }
        p->offset++;
        goto amp_try_again;
    }
    else if (amp_is_format(current))    {
        amp_get_format(p);
        goto amp_try_again;
    }
    else if (current == '-' && p->indentation && !(p->state & AMP_IS_SEEN_CONTENT) && amp_is_space(amp_next(p, 1)))           
                                        return amp_get_list     (p);
    else if (current == '#')            return amp_get_header   (p);
    else                                return amp_get_text     (p);

    return t;
}

int main(void) {
    amp_Parser mp = {
        .source = amp_slice("##Hello\nMy `name` ==~~**is**~~ joshua== :o\n - Hi! - las\n  - Rust")
    };

    amp_Token t = {0};

    while( (t = amp_get_token(&mp)).type) {
        printf("%i\t", mp.current_type);
        printf("TOKEN: %i:%8x:%i:%i:'%.*s'\n", 
                t.type, 
                t.properties,
                t.header_size,
                t.indentation,
                (int)t.text.length, t.text.data
        );
    }

}
