#include "../src/memtracker.h"
#include "string.h"

enum {
    HC_MT_MALLOC,
    HC_MT_ARENA,
    HC_MT_MMAP,
    HC_MT_WINMEMMAP,
};


int main(void) {
    hc_MemoryTracker tracker = {0};

    int N = 64;
    void* pointers[N];

    // allocate a lot of memory
    for(int i = 0; i < N; i++)
        pointers[i] = hc_mtracker_put_dynamic(&tracker, malloc(1024), HC_MT_MALLOC, 0);

    // make sure all of it free-d in order.
    hc_tracker_free_match(&tracker, it) {
        case HC_MT_MALLOC: free(it); break;
    }
    free(tracker.items);
}
