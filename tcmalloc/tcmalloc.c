#include "tcmalloc.h"
#include <stdint.h>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include "list.h"
#include "common.h"
#include "object.h"
#include "page_map.h"
#include "span.h"
#include "page_heap.h"
#include "central_free_list.h"
#include "thread_cache.h"

#undef DEBUG_PRINT

// TODO: Figure out optimum batch size etc.
// TODO: Use better allocation mechanism for spans etc. Also should consider zeroing
// TODO: Use inline?

void *tc_central_init() {
    if (pthread_spin_init(&global_lock, PTHREAD_PROCESS_PRIVATE) != 0
        || page_map_new() == -1
        || page_heap_new() == -1
        || central_free_list_new() == -1
    ) {
        return NULL;
    }
    return central_free_list;
}

void *tc_thread_init() {
    if (thread_cache_new() == -1) {
        return NULL;
    }
    return thread_cache;
}

// TODO: Debug print when error happened?

void *tc_malloc(size_t size) {
    void *ptr;
    
    if (size == 0) {
        ptr = NULL;
    } else if (size <= SMALL_OBJECT_SIZE_LIMIT) {
        size_t class = class_from_size(size);
        struct object *object = thread_cache_get_object(class);
        if (object == NULL) {
            return NULL;
        }
        ptr = object;
    } else {
        size_t num_pages = (size - 1) / PAGE_SIZE + 1;
        pthread_spin_lock(&global_lock);
        struct span *span = page_heap_get_span(num_pages);
        if (span == NULL) {
            pthread_spin_unlock(&global_lock);
            return NULL;
        }
        span->state = SPAN_ALLOCATED_LARGE;
        pthread_spin_unlock(&global_lock);
        ptr = span->page_start;
    }

#ifdef DEBUG_PRINT
    printf("********** tc_malloc(%zu) -> %p **********\n\n", size, ptr);
    tc_debug_print();
    printf("\n");
#endif

    return ptr;
}

void tc_free(void *ptr) {
    if (ptr == NULL) {
        return;
    }

    size_t page_number = (uintptr_t)ptr / PAGE_SIZE;
    struct span *span = page_map_get(page_number);
    if (span == NULL) {
        return;
    }

    switch (span->state) {
    case SPAN_ALLOCATED_SMALL:
        thread_cache_insert(span->object_class, ptr);
        break;
    case SPAN_ALLOCATED_LARGE:
        pthread_spin_lock(&global_lock);
        span_coalesce(span);
        page_heap_insert_span(span);
        pthread_spin_unlock(&global_lock);
        break;
    case SPAN_FREE:
        assert(false);
        return;
    }


#ifdef DEBUG_PRINT
    printf("********** free(%p) **********\n\n", ptr);
    tc_debug_print();
    printf("\n");
#endif
}

void tc_debug_print() {
    pthread_spin_lock(&global_lock);

    page_map_print();
    page_heap_print();
    central_free_list_print();
    thread_cache_print();

    pthread_spin_unlock(&global_lock);
}
