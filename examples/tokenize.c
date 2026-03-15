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
        tkn_entry("%s"),
    };
    while(token = str_chop_token_or_string(&src, tokens, 1, "\"","\""), 
            tkn_is_valid(token)) 
        printf("\t token: '%s',\t kind: %u\n", tkn_to_cstring(token), token.kind);
    
}
