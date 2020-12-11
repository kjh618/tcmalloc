#pragma once
#include <stddef.h>
#include <stdbool.h>

struct object {
    struct object *next;
};

struct object_list {
    struct object *head;
    size_t length;
    size_t low_mark;
};

void object_list_init(struct object_list *list);

bool object_list_is_empty(struct object_list *list);

void object_list_push_front(struct object_list *list, struct object *object);
struct object *object_list_pop_front(struct object_list *list);

size_t object_list_count(struct object_list *list);
