#pragma once
#include <stddef.h>

void *tc_central_init();
void *tc_thread_init();
void *tc_malloc(size_t size);
void tc_free(void *ptr);

void tc_debug_print();
