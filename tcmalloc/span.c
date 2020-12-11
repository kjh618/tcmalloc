#include "span.h"
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include "list.h"
#include "common.h"
#include "object.h"
#include "page_map.h"

// TODO: Change API so that including `page_map.h` is unnecessary

static struct span *span_new_with(void *page_start, size_t num_pages) {
    assert(1 <= num_pages && num_pages <= MAX_NUM_PAGES);

    struct span *span = sbrk(sizeof(struct span));
    if (span == NULL) {
        return NULL;
    }

    span->link.next = NULL;
    span->link.prev = NULL;

    span->page_start = page_start;
    span->num_pages = num_pages;

    span->state = SPAN_FREE;

    span->object_class = 0;
    object_list_init(&span->free_objects);

    if (page_map_insert(span) == -1) {
        sbrk(-sizeof(struct span));
        return NULL;
    }

    return span;
}

struct span *span_new(size_t num_pages) {
    assert(1 <= num_pages && num_pages <= MAX_NUM_PAGES);

    void *page_start = mmap_anonymous(num_pages * PAGE_SIZE);
    if (page_start == NULL) {
        return NULL;
    }

    return span_new_with(page_start, num_pages);
}

bool span_is_consistent(struct span *span) {
    // TODO: Check consistency of other data structures as well?

    bool ret = 1 <= span->num_pages && span->num_pages <= MAX_NUM_PAGES;

    if (span->state == SPAN_ALLOCATED_SMALL) {
        ret = ret && span->object_class < NUM_CLASSES;
    }

    return ret;
}

struct span *span_split(struct span *span, size_t num_pages) {
    assert(span->state == SPAN_FREE);
    assert(1 <= num_pages && num_pages <= MAX_NUM_PAGES);

    page_map_remove(span);
    span->num_pages -= num_pages;
    assert(1 <= span->num_pages && span->num_pages <= MAX_NUM_PAGES);
    page_map_insert(span);

    return span_new_with((char *)span->page_start + span->num_pages * PAGE_SIZE, num_pages);
}

void span_make_objects(struct span *span, size_t object_class) {
    assert(span->state == SPAN_FREE);
    assert(object_class < NUM_CLASSES);

    span->state = SPAN_ALLOCATED_SMALL;
    size_t object_size = class_get_size(object_class);
    for (struct object *object = span->page_start;
        (char *)object + object_size <= (char *)span->page_start + span->num_pages * PAGE_SIZE;
        object = (struct object *)((char *)object + object_size)
    ) {
        object_list_push_front(&span->free_objects, object);
    }
    span->object_class = object_class;
}

static void span_merge_with(struct span *span, struct span *other, bool is_before) {
    if (is_before) {
        span->page_start = other->page_start;
    }
    span->num_pages += other->num_pages;
    list_remove(&other->link); // Remove from `page_heap`
    page_map_remove(other);
    // TODO: Free `other`
}

void span_coalesce(struct span *span) {
    page_map_remove(span);

    size_t page_number = (uintptr_t)span->page_start / PAGE_SIZE;

    size_t page_number_before = page_number - 1;
    struct span *span_before = page_map_get(page_number_before);
    if (span_before != NULL && span_before->state == SPAN_FREE) {
        span_merge_with(span, span_before, true);
    }

    size_t page_number_after = page_number + span->num_pages;
    struct span *span_after = page_map_get(page_number_after);
    if (span_after != NULL && span_after->state == SPAN_FREE) {
        span_merge_with(span, span_after, false);
    }

    page_map_insert(span);
}

void span_print(struct span *span) {
    assert(span_is_consistent(span));

    printf("{ %zu pages from %p, state %d, (%zu objects of class %zd) }",
        span->num_pages, span->page_start,
        span->state,
        object_list_count(&span->free_objects), span->object_class
    );
}
