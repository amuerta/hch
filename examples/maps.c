
#define NOB_IMPLEMENTATION

#define INCLUDE_MAP
#define HCH_MAP_IMPLEMENTATION

#include "../nob.h"
#include "../hc.h"

typedef struct {
    Nob_String_View word;
    int count;
} Word;

typedef struct {
    Word* items;
    size_t count, capacity;
} Words;

typedef struct {
    Map head;
    int* items;
} CountMap;

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
    Map* map = &m->head;

    if (map_load(*map) > 0.75) {
        int* new_items = calloc(map->capacity*2, sizeof(*m->items));
        map_resize(map, map->capacity*2, {
                new_items[newid] = m->items[oldid];
                });
        free(m->items);
        m->items = new_items;
    }
}

void count_words_map(Words all, CountMap* m) {
    Nob_String_View v;
    for(size_t i = 0; i < all.count; i++) {
        v = all.items[i].word;
        if (!v.count) continue;
        Map* map = &m->head;
    
        resize_words_map_if_needed(m);
    
        long int index = map_query(*map, map_slice(v.data, v.count));
        if (index == -1) {
            long int new = map_reserve(map, map_slice(v.data, v.count));
            assert(new != -1 && "map is full");
            m->items[new] = 0;
        } else {
            m->items[index]++;
        }

        nob_temp_reset();
    }
}


#define COUNT 1024*64

int main(void) {
    CountMap m = {
        .items = calloc(COUNT, sizeof(int)),
        .head = map_alloc(0, 0)
    };

    Words w = {0};
    Words f = {0};

    Nob_String_Builder sb = {0};
    if(!nob_read_entire_file("./files/pg100.txt", &sb)) {
        nob_log(NOB_INFO, "FAILED TO READ A FILE"); 
        return -1;
    }

    split_to_words(&w, sb);

    printf("split into = %lu slices\n", w.count);

    // ~1000x speedup!
    //count_words_linear(&w, &f); // ~30-40 seconds
    count_words_map(w, &m); // ~30-40 ms

    printf("linear.count = %lu\n", f.count);
    printf("map.count = %lu\n", m.head.count);

    // i know the correct count
    assert(m.head.count == 49820);

    // TODO: show both methods results
    // (i.m. sort using count and print result)
    // TODO: aslo measure time
    printf("if this has no assert, it means map did its job correctly\n");

    nob_sb_free(sb);

    free(w.items);
    free(f.items);
    
    
    map_clear(&m.head);
    free(m.items);
    // VIMRC: Normal/Visual mode tab is comment toggle
    //list_test(); 
    //slice_test();
 
}
