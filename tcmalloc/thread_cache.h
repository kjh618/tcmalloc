#pragma once
#include <stddef.h>

struct thread_cache;

extern __thread struct thread_cache *thread_cache;

int thread_cache_new();

struct object *thread_cache_get_object(size_t class);
void thread_cache_insert(size_t class, struct object *object);
void thread_cache_gc();

void thread_cache_print();
