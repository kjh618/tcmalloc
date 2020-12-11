#pragma once
#include <stddef.h>
#include "list.h"
#include "object.h"

enum span_state {
    SPAN_FREE,
    SPAN_ALLOCATED_SMALL,
    SPAN_ALLOCATED_LARGE
};

struct span {
    struct list_link link;

    void *page_start;
    size_t num_pages;

    enum span_state state;

    size_t object_class;
    struct object_list free_objects;
};

struct span *span_new(size_t num_pages);

bool span_is_consistent(struct span *span);

struct span *span_split(struct span *span, size_t num_pages);
void span_make_objects(struct span *span, size_t object_class);
void span_coalesce(struct span *span);

void span_print(struct span *span);
