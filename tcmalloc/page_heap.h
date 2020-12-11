#pragma once
#include "list.h"
#include "span.h"

#define PAGE_HEAP_ARRAY_SIZE 256

struct page_heap;

extern struct page_heap *page_heap;

int page_heap_new();

struct span *page_heap_get_span(size_t num_pages);
void page_heap_insert_span(struct span *span);

void page_heap_print();
