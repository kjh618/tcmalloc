#include "list.h"
#include <stdio.h>

void list_init(struct list *list) {
    list->head.next = &list->tail;
    list->head.prev = NULL;
    list->tail.next = NULL;
    list->tail.prev = &list->head;
}

struct list_link *list_begin(struct list *list) {
    return list->head.next;
}

struct list_link *list_end(struct list *list) {
    return &list->tail;
}

bool list_is_empty(struct list *list) {
    return list_begin(list) == list_end(list);
}

void list_push_front(struct list *list, struct list_link *link) {
    link->next = list->head.next;
    link->prev = &list->head;

    list->head.next->prev = link;
    list->head.next = link;
}

void list_remove(struct list_link *link) {
    link->next->prev = link->prev;
    link->prev->next = link->next;

    link->next = NULL;
    link->prev = NULL;
}

struct list_link *list_pop_front(struct list *list) {
    struct list_link *begin = list_begin(list);
    list_remove(begin);
    return begin;
}

void list_print(struct list *list, void (*elem_print)(struct list_link *link)) {
    for (struct list_link *link = list_begin(list); link != list_end(list); link = link->next) {
        if (link != list_begin(list)) {
            printf("    ");
        }
        printf("<=> ");
        elem_print(link);
        printf("\n");
    }
}

size_t list_count(struct list *list) {
    size_t count = 0;
    for (struct list_link *link = list_begin(list); link != list_end(list); link = link->next) {
        count++;
    }
    return count;
}
