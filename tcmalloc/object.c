#include "object.h"
#include <assert.h>

void object_list_init(struct object_list *list) {
    list->head = NULL;
    list->length = 0;
    list->low_mark = 0;
}

bool object_list_is_empty(struct object_list *list) {
    return list->head == NULL;
}

void object_list_push_front(struct object_list *list, struct object *object) {
    object->next = list->head;
    list->head = object;

    list->length++;
}

struct object *object_list_pop_front(struct object_list *list) {
    if (list->head == NULL) {
        return NULL;
    } else {
        struct object *old_head = list->head;
        list->head = old_head->next;
        old_head->next = NULL;

        list->length--;
        if (list->length < list->low_mark) {
            list->low_mark = list->length;
        }

        return old_head;
    }
}

size_t object_list_count(struct object_list *list) {
    size_t count = 0;
    for (struct object *object = list->head; object != NULL; object = object->next) {
        count++;
    }

    assert(count == list->length);

    return count;
}
