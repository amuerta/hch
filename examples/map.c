#include "../src/map.h"
#define Map hc_Map

typedef Map(int) IntMap;

#define             GLOBAL_MAP_SIZE 256
static MapKeySlice  GLOBAL_MAP_KEYS [GLOBAL_MAP_SIZE];
static int          GLOBAL_MAP_ITEMS[GLOBAL_MAP_SIZE];

IntMap global_map(void) {
    IntMap map = {
        .items = GLOBAL_MAP_ITEMS,
        .map_head = {
            .capacity = GLOBAL_MAP_SIZE,
            .keys     = GLOBAL_MAP_KEYS,
            .typesize = sizeof(*GLOBAL_MAP_ITEMS),
            .heap_allocated = false,
        },
    };
    hc_map_set_default_hashes(&map.map_head);
    return map;
}


void map_put(IntMap* map, int value, const char* key) {
    int *item;
    item = hc_map_ref_or_reserve(map, hc_map_key(key));
    *item = value;
}

void map_assert_key_value(IntMap* m, int value, const char* key) {
    int* item;
    item = hc_map_ref(m, hc_map_key(key));
    printf("> making sure '%s' exists with %i ", key, value);
    assert(*item == value);
    printf(".. OK\n");
}

void test_map_global(void) {
    IntMap gmap = global_map();   

    map_put(&gmap, 420, "funny1");
    map_put(&gmap, 13377, "funny2");
    map_put(&gmap, 69, "funny3");

    map_assert_key_value(&gmap, 420,    "funny1");
    map_assert_key_value(&gmap, 13377,  "funny2");
    map_assert_key_value(&gmap, 69,     "funny3");

    assert(!hc_map_ref(&gmap, hc_map_key("funny4")));
    assert(!hc_map_ref(&gmap, hc_map_key("funny_3")));
    assert(!hc_map_ref(&gmap, hc_map_key("funn3")));
}

void test_map_heap(void) {
    int count = 2;
    IntMap map = {
        .items = calloc         (count * sizeof(int) , 1),
        .map_head = hc_map_heap (count,  sizeof(int))
    };

    map_put(&map, 1, "1");
    map_put(&map, 2, "2");

    hc_map_grow(&map, 8);

    map_put(&map, 3, "3");
    map_put(&map, 4, "4");
    map_put(&map, 5, "5");
    map_put(&map, 6, "6");
    map_put(&map, 7, "7");
    map_put(&map, 8, "8");


    // print all key+value pairs to show new map
    for(size_t i = 0; i < map.map_head.capacity; i++) {
        MapKeySlice key = map.map_head.keys[i];
        if(hc_map_key_is_empty(key)) continue;

        printf("#%li   '%.*s': %i\n",
                i, 
                hc_map_key_fmt(map.map_head.keys[i]), 
                map.items[i]);
    }

    hc_map_free(&map);
}

int main(void) {
    test_map_global();
    test_map_heap();
}
