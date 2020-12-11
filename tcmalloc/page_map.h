#pragma once
#include <stddef.h>

struct page_map;

extern struct page_map *page_map;

int page_map_new();

struct span *page_map_get(size_t page_number);
int page_map_insert(struct span *span);
void page_map_remove(struct span *span);

void page_map_print();
