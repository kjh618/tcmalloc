#pragma once
#include <stdbool.h>
#include <stddef.h>

/* Embed `struct list_link` in a struct to create a doubly linked list.
 * If the link is the first field of a struct, you can just cast the pointer to
 * the link to get the outer struct, without using the `list_entry` macro.
 */
struct list_link {
    struct list_link *next;
    struct list_link *prev;
};

/* To significantly reduce special cases, a list always has a head and a tail
 * which are not "real" elements.
 * An empty list looks like the following:
 * 0 <- [head] <=> [tail] -> 0
 * And a list with 3 elements looks like this:
 * 0 <- [head] <=> [1] <=> [2] <=> [3] <=> [tail] -> 0
 *
 * To iterate through all elements in a list, use the following template:
 * ```
 * for (struct list_link *link = list_begin(list); link != list_end(list); link = link->next) {
 *     // ...
 * }
 * ```
 */
struct list {
    struct list_link head;
    struct list_link tail;
};

void list_init(struct list *list);

struct list_link *list_begin(struct list *list);
struct list_link *list_end(struct list *list);
bool list_is_empty(struct list *list);

void list_push_front(struct list *list, struct list_link *link);
void list_remove(struct list_link *link);
struct list_link *list_pop_front(struct list *list);

void list_print(struct list *list, void (*elem_print)(struct list_link *link));
size_t list_count(struct list *list);
