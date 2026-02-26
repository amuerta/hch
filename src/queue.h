#ifndef __HC_QUEUE_H
#define __HC_QUEUE_H

/*
 * Queue superset for array, exists to self-document code and 
 * annoy you with bloat (BOO HOo).
 */

#define hc_Queue(T) struct {                \
    T* items;                               \
    size_t count, capacity, queue_pointer;  \
}                           

#define que_typesize(queue) sizeof((queue)->items[0])

#define que_pop(queue) \
    ((queue)->items[que_pop_helper(queue)])

#define que_push(queue, ITEM) \
    que_push_ex(queue, que_typesize(queue), ITEM, sizeof( *(ITEM) ))

typedef struct {
    void* items;
    size_t count, capacity, queue_pointer;
} hc_QueueBase;

size_t que_id(void* queue, size_t i) {
    hc_QueueBase* q = queue;
    return((q->queue_pointer + i) % q->capacity);
}

size_t que_pop_helper(void* queue) {
    hc_QueueBase* q = queue;
    size_t it = que_id(queue, 0);
    q->count--;
    q->queue_pointer++;
    return it;
}

void que_push_ex(void* queue, size_t typesize, void* item, size_t item_size) {
    hc_QueueBase* q = queue;
    assert(typesize == item_size);
    
    void* ptr = q->items + ((q->queue_pointer + q->count) % q->capacity)* typesize;
    memcpy(ptr, item, item_size);

    if(q->count >= q->capacity) 
        q->queue_pointer++;
    else 
        q->count++;
}

#endif /* __HC_QUEUE_H */
