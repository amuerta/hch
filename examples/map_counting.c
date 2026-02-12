
#define NOB_IMPLEMENTATION

#include "../nob.h"

// legacy non header based dynamic array
#define HC_DA_MACRO_BASED
#include "../src/da.h"
#include "../src/map.h"

#define da_append hc_da_append
#define Map hc_Map

typedef struct {
    Nob_String_View word;
    int count;
} Word;

typedef struct {
    Word* items;
    size_t count, capacity;
} Words;

typedef Map(int) CountMap;

// count interative
void split_to_words(Words* all, Nob_String_Builder text) {
    Nob_String_View slice = nob_sb_to_sv(text);
    Nob_String_View crop;
    
    while(slice.count) {
        crop = nob_sv_chop_by_delim(&slice, ' ');
        //printf(SV_Fmt"\n", (int)crop.count, crop.data);
        Word w = {
            .word = crop,
            .count = 0
        };
        da_append(all,w);
    }
}

void count_words_linear(Words* all, Words* found) {
    for(size_t i = 0; i < all->count; i++) {
        bool saw = false;
        Word current = all->items[i];
        for(size_t j = 0; j < found->count; j++) {
            Word seen = found->items[j];
            if (nob_sv_eq(current.word, seen.word)) {
                found->items[j].count++;
                saw = true;
            }
        }
        if (!saw) {
            da_append(found, current);
        }
    }
}



void resize_words_map_if_needed(CountMap* m) {

    if (hc_map_load(m->map_head) > 0.75) {
        assert(!"TODO");
    }
}

void count_words_map(Words all, CountMap* m) {
    Nob_String_View v;
    for(size_t i = 0; i < all.count; i++) {
        v = all.items[i].word;
        if (!v.count) continue;
        // MapHead* map = &m->map_head;
    
        resize_words_map_if_needed(m);
    
        MapKeySlice key = hc_map_slice(v.data, v.count);
        int* n = hc_map_ref_or_reserve(m, key);
        (*n)++;

        nob_temp_reset();
    }
}


#define COUNT 1024*128

int main(void) {
    CountMap m = {
        .items = calloc(COUNT, sizeof(int)),
        .map_head = hc_map_heap(COUNT, sizeof(int))
    };

    Words w = {0};
    Words f = {0};

    Nob_String_Builder sb = {0};
    if(!nob_read_entire_file("./examples/files/pg100.txt", &sb)) {
        nob_log(NOB_INFO, "FAILED TO READ A FILE"); 
        return -1;
    }

    split_to_words(&w, sb);

    printf("split into = %lu slices\n", w.count);

    // ~1000x speedup! (on ryzen 5 5625u)
    //count_words_linear(&w, &f); // ~30-40 seconds
    count_words_map(w, &m); // ~30-40 ms

    printf("linear.count = %lu\n", f.count);
    printf("map.count = %lu\n", m.map_head.count);

    // i know the correct count
    assert(m.map_head.count == 49820);

    // TODO: show both methods results
    // (i.m. sort using count and print result)
    // TODO: aslo measure time
    printf("if no assert, then it means map did its job correctly\n");

    nob_sb_free(sb);

    free(w.items);
    free(f.items);
    
    
    hc_map_free(&m);
    //list_test(); 
    //slice_test();
 
}
