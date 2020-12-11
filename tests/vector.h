#pragma once
#include <stddef.h>
#include <stdbool.h>

struct vector;

struct vector *vector_new(size_t elem_size, bool doubling, size_t initial_capacity);
void vector_free(struct vector *vec, void (*ptr_elem_free)(void *ptr_elem));
int vector_shrink_to_fit(struct vector *vec);
void *vector_into_array(struct vector *vec);

size_t vector_size(struct vector *vec);

/* Return a pointer to the element at `index`. */
void *vector_ptr_at(struct vector *vec, size_t index);
/* Copy `*ptr_elem` bit by bit into the vector. */
int vector_push(struct vector *vec, const void *ptr_elem);
void vector_iter(struct vector *vec, void (*ptr_elem_func)(void *ptr_elem));
