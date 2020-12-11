#include "central_free_list.h"
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include "list.h"
#include "common.h"
#include "span.h"
#include "page_heap.h"

struct central_free_list {
    struct list spans[NUM_CLASSES];
};

struct central_free_list *central_free_list;

int central_free_list_new() {
    central_free_list = sbrk(sizeof(struct central_free_list));
    if (central_free_list == NULL) {
        return -1;
    }
    for (size_t class = 0; class < NUM_CLASSES; class ++) {
        list_init(&central_free_list->spans[class]);
    }
    return 0;
}

static int central_free_list_fill(size_t class) {
    assert(class < NUM_CLASSES);

    struct list *spans = &central_free_list->spans[class];
    size_t num_pages = class_get_size(class) * BATCH_SIZE / PAGE_SIZE;
    if (num_pages < 1) {
        num_pages = 1;
    }

    struct span *free_span = page_heap_get_span(num_pages);
    if (free_span == NULL) {
        return -1;
    }
    span_make_objects(free_span, class);
    list_push_front(spans, &free_span->link);
    return 0;
}

ssize_t central_free_list_get_objects(size_t class, struct object_list *free_objects, size_t batch_size) {
    assert(class < NUM_CLASSES);
    assert(batch_size > 0);

    struct list *spans = &central_free_list->spans[class];

    size_t count = 0;
    for (struct list_link *link = list_begin(spans);
        link != list_end(spans);
        link = link->next
    ) {
        struct span *span = (struct span *)link;
        while (!object_list_is_empty(&span->free_objects) && count < batch_size) {
            struct object *free_object = object_list_pop_front(&span->free_objects);
            object_list_push_front(free_objects, free_object);
            count++;
        }
    }

    if (count == 0) {
        if (central_free_list_fill(class) == -1) {
            return -1;
        }

        return central_free_list_get_objects(class, free_objects, batch_size);
    }

    return count;
}

void central_free_list_insert_object(size_t class, struct object *object) {
    assert(class < NUM_CLASSES);

    struct list *spans = &central_free_list->spans[class];
    struct span *span = NULL;
    for (struct list_link *link = list_begin(spans); link != list_end(spans); link = link->next) {
        span = (struct span *)link;
        if (span->page_start <= (void *)object
            && (char *)object < (char *)span->page_start + span->num_pages * PAGE_SIZE
        ) {
            break;
        }
    }
    assert(span != NULL);

    object_list_push_front(&span->free_objects, object);

    if ((span->free_objects.length + 1) * class_get_size(span->object_class) > span->num_pages * PAGE_SIZE) {
        list_remove(&span->link); // Remove from `central_free_list`
        span_coalesce(span);
        page_heap_insert_span(span);
    }
}

static void span_print_from_link(struct list_link *span_link) {
    struct span *span = (struct span *)span_link;
    span_print(span);
}

void central_free_list_print() {
    printf("central_free_list->spans\n");
    for (size_t class = 0; class < NUM_CLASSES; class++) {
        struct list *spans = &central_free_list->spans[class];
        if (!list_is_empty(spans)) {
            printf("  [class %zu] ", class);
            list_print(spans, span_print_from_link);
        }
    }
    printf("\n");
}
