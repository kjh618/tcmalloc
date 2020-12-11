#include "page_heap.h"
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include "common.h"

/* `page_heap->free_spans[i]` contains spans with `num_pages == i + 1`.
 * Except the last element `page_heap->free_spans[255]`, which contains spans
 * with `num_pages >= 256`.
 */
struct page_heap {
    struct list free_spans[PAGE_HEAP_ARRAY_SIZE];
};

struct page_heap *page_heap;

int page_heap_new() {
    page_heap = sbrk(sizeof(struct page_heap));
    if (page_heap == NULL) {
        return -1;
    }
    for (size_t i = 0; i < PAGE_HEAP_ARRAY_SIZE; i++) {
        list_init(&page_heap->free_spans[i]);
    }
    return 0;
}

static int page_heap_fill(size_t num_pages) {
    assert(1 <= num_pages && num_pages <= MAX_NUM_PAGES);

    // Find a larger span in [0] ~ [254] and split
    for (size_t i = num_pages + 1 - 1; i < 255; i++) {
        struct list *free_spans = &page_heap->free_spans[i];
        if (!list_is_empty(free_spans)) {
            struct span *span = (struct span *)list_pop_front(free_spans);
            struct span *new_span = span_split(span, num_pages);
            if (new_span == NULL) {
                return -1;
            }

            page_heap_insert_span(span);
            page_heap_insert_span(new_span);

            return 0;
        }
    }

    // Find a larger span in [255] and split
    struct list *free_spans = &page_heap->free_spans[PAGE_HEAP_ARRAY_SIZE - 1];
    if (!list_is_empty(free_spans)) {
        for (struct list_link *link = list_begin(free_spans);
            link != list_end(free_spans);
            link = link->next
        ) {
            struct span *span = (struct span *)link;
            if (span->num_pages > num_pages) {
                list_remove(&span->link);
                struct span *new_span = span_split(span, num_pages);
                if (new_span == NULL) {
                    return -1;
                }

                page_heap_insert_span(span);
                page_heap_insert_span(new_span);

                return 0;
            }
        }
    }

    // No larger span found; allocate new pages/span
    struct span *new_span = span_new(num_pages);
    if (new_span == NULL) {
        return -1;
    }
    page_heap_insert_span(new_span);

    return 0;
}

struct span *page_heap_get_span(size_t num_pages) {
    assert(1 <= num_pages && num_pages <= MAX_NUM_PAGES);

    if (num_pages < 256) {
        struct list *free_spans = &page_heap->free_spans[num_pages - 1];
        if (list_is_empty(free_spans)) {
            if (page_heap_fill(num_pages) == -1) {
                return NULL;
            }
        }

        return (struct span *)list_pop_front(free_spans);
    } else {
        struct list *free_spans = &page_heap->free_spans[PAGE_HEAP_ARRAY_SIZE - 1];
        if (list_is_empty(free_spans)) {
            if (page_heap_fill(num_pages) == -1) {
                return NULL;
            }
        }

        // Find span with `num_pages`
        for (struct list_link *link = list_begin(free_spans);
            link != list_end(free_spans);
            link = link->next
        ) {
            struct span *span = (struct span *)link;
            if (span->num_pages == num_pages) {
                list_remove(&span->link);

                return span;
            }
        }

        // No span with `num_pages` found; Fill and retry
        if (page_heap_fill(num_pages) == -1) {
            return NULL;
        }

        return page_heap_get_span(num_pages);
    }
}

void page_heap_insert_span(struct span *span) {
    size_t i = span->num_pages - 1;
    if (i >= PAGE_HEAP_ARRAY_SIZE) {
        i = PAGE_HEAP_ARRAY_SIZE - 1;
    }

    list_push_front(&page_heap->free_spans[i], &span->link);
    span->state = SPAN_FREE;
    span->object_class = 0;
    object_list_init(&span->free_objects);
}

void page_heap_print() {
    printf("page_heap->free_spans\n");
    for (size_t i = 0; i < PAGE_HEAP_ARRAY_SIZE; i++) {
        struct list *free_spans = &page_heap->free_spans[i];
        if (!list_is_empty(free_spans)) {
            printf("  [%zu] <=> (%zu spans)\n", i, list_count(free_spans));
        }
    }
    printf("\n");
}
