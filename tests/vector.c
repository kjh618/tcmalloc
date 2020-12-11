#include "vector.h"
#include <string.h>
#include "../tcmalloc/tcmalloc.h"

struct vector {
    size_t elem_size;
    bool doubling;

    void *array;
    size_t size;
    size_t capacity;
};

struct vector *vector_new(size_t elem_size, bool doubling, size_t initial_capacity) {
    struct vector *vec = tc_malloc(sizeof(struct vector));
    if (vec == NULL) {
        return NULL;
    }
    vec->elem_size = elem_size;
    vec->doubling = doubling;
    vec->size = 0;
    vec->capacity = initial_capacity;
    vec->array = tc_malloc(vec->capacity * vec->elem_size);
    if (vec->array == NULL) {
        tc_free(vec);
        return NULL;
    }

    return vec;
}

void vector_free(struct vector *vec, void (*ptr_elem_free)(void *ptr_elem)) {
    if (ptr_elem_free != NULL) {
        vector_iter(vec, ptr_elem_free);
    }
    tc_free(vec->array);
    tc_free(vec);
}

static int vector_change_capacity(struct vector *vec, size_t new_capacity) {
    void *new_array = tc_malloc(new_capacity * vec->elem_size);
    if (new_array == NULL) {
        return -1;
    }
    memcpy(new_array, vec->array, vec->size * vec->elem_size);
    tc_free(vec->array);
    vec->array = new_array;
    vec->capacity = new_capacity;
    return 0;
}

int vector_shrink_to_fit(struct vector *vec) {
    vec->capacity = vec->size;
    return vector_change_capacity(vec, vec->capacity);
}

void *vector_into_array(struct vector *vec) {
    void *array = vec->array;
    tc_free(vec);
    return array;
}

size_t vector_size(struct vector *vec) {
    return vec->size;
}

void *vector_ptr_at(struct vector *vec, size_t index) {
    return vec->array + index * vec->elem_size;
}

int vector_push(struct vector *vec, const void *ptr_elem) {
    if (vec->size >= vec->capacity) {
        size_t new_capacity = vec->capacity;
        if (vec->doubling) {
            new_capacity *= 2;
        } else {
            new_capacity++;
        }

        if (vector_change_capacity(vec, new_capacity) == -1) {
            return -1;
        }
    }

    memcpy(vector_ptr_at(vec, vec->size), ptr_elem, vec->elem_size);
    vec->size++;
    return 0;
}

void vector_iter(struct vector *vec, void (*ptr_elem_func)(void *ptr_elem)) {
    for (size_t i = 0; i < vec->size; i++) {
        void *ptr_elem = vector_ptr_at(vec, i);
        ptr_elem_func(ptr_elem);
    }
}
