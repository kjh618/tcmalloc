#include "thread_cache.h"
#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include "common.h"
#include "object.h"
#include "central_free_list.h"

#define THREAD_CACHE_GC_LIMIT ((size_t)1 << 10)

struct thread_cache {
    struct object_list free_objects[NUM_CLASSES];
    size_t total_size;
};

__thread struct thread_cache *thread_cache;

int thread_cache_new() {
    thread_cache = mmap_anonymous(sizeof(struct thread_cache));
    if (thread_cache == NULL) {
        return -1;
    }
    for (size_t class = 0; class < NUM_CLASSES; class ++) {
        object_list_init(&thread_cache->free_objects[class]);
    }
    thread_cache->total_size = 0;
    return 0;
}

static int thread_cache_fill(size_t class) {
    assert(class < NUM_CLASSES);

    struct object_list *free_objects = &thread_cache->free_objects[class];
    pthread_spin_lock(&global_lock);
    ssize_t count = central_free_list_get_objects(class, free_objects, BATCH_SIZE);
    pthread_spin_unlock(&global_lock);
    if (count == -1) {
        return -1;
    }
    thread_cache->total_size += class_get_size(class) * count;
    return 0;
}

struct object *thread_cache_get_object(size_t class) {
    assert(class < NUM_CLASSES);

    struct object_list *free_objects = &thread_cache->free_objects[class];
    if (object_list_is_empty(free_objects)) {
        if (thread_cache_fill(class) == -1) {
            return NULL;
        }
    }

    thread_cache->total_size -= class_get_size(class);
    return object_list_pop_front(free_objects);
}

void thread_cache_insert(size_t class, struct object *object) {
    assert(class < NUM_CLASSES);

    struct object_list *free_objects = &thread_cache->free_objects[class];
    object_list_push_front(free_objects, object);
    thread_cache->total_size += class_get_size(class);

    if (thread_cache->total_size > THREAD_CACHE_GC_LIMIT) {
        thread_cache_gc();
    }
}

void thread_cache_gc() {
    for (size_t class = 0; class < NUM_CLASSES; class++) {
        struct object_list *free_objects = &thread_cache->free_objects[class];
        size_t object_size = class_get_size(class);
        for (size_t i = 0; i < free_objects->low_mark / 2; i++) {
            struct object *object = object_list_pop_front(free_objects);
            thread_cache->total_size -= object_size;
            pthread_spin_lock(&global_lock);
            central_free_list_insert_object(class, object);
            pthread_spin_unlock(&global_lock);
        }

        free_objects->low_mark = free_objects->length;
    }
}

static size_t thread_cache_calculate_total_size() {
    size_t total_size = 0;
    for (size_t class = 0; class < NUM_CLASSES; class++) {
        struct object_list *free_objects = &thread_cache->free_objects[class];
        total_size += free_objects->length * class_get_size(class);
    }
    return total_size;
}

void thread_cache_print() {
    printf("thread_cache->free_objects\n");
    for (size_t class = 0; class < NUM_CLASSES; class++) {
        struct object_list *free_objects = &thread_cache->free_objects[class];
        if (!object_list_is_empty(free_objects)) {
            printf("  [class %zu] -> (%zu objects, low mark %zu)\n",
                class,
                object_list_count(free_objects), free_objects->low_mark
            );
        }
    }
    printf("thread_cache->total_size: %zu (should be %zu)\n",
        thread_cache->total_size,
        thread_cache_calculate_total_size()
    );
    printf("\n");
}
