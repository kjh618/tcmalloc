#include "page_map.h"
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <assert.h>
#include "common.h"
#include "span.h"

#define RADIX_TREE_NUM_BITS 18
#define RADIX_TREE_NODE_SIZE ((size_t)1 << RADIX_TREE_NUM_BITS)
#define RADIX_TREE_MASK (RADIX_TREE_NODE_SIZE - 1)

struct radix_tree_node {
    void *children[RADIX_TREE_NODE_SIZE];
};

struct page_map {
    struct radix_tree_node root;
};

struct page_map *page_map;

static void radix_tree_node_init(struct radix_tree_node *node) {
    for (size_t i = 0; i < RADIX_TREE_NODE_SIZE; i++) {
        node->children[i] = NULL;
    }
}

int page_map_new() {
    page_map = sbrk(sizeof(struct page_map));
    if (page_map == NULL) {
        return -1;
    }
    radix_tree_node_init(&page_map->root);
    return 0;
}

static size_t radix_tree_get_index(size_t page_number, size_t level) {
    assert(page_number < MAX_NUM_PAGES);
    assert(1 <= level && level <= 3);
    return (page_number >> (RADIX_TREE_NUM_BITS * (3 - level))) & RADIX_TREE_MASK;
}

static void **page_map_get_ptr(size_t page_number) {
    assert(page_number < MAX_NUM_PAGES);

    size_t i1 = radix_tree_get_index(page_number, 1);
    struct radix_tree_node *l2 = page_map->root.children[i1];
    if (l2 == NULL) {
        return NULL;
    }

    size_t i2 = radix_tree_get_index(page_number, 2);
    struct radix_tree_node *l3 = l2->children[i2];
    if (l3 == NULL) {
        return NULL;
    }

    size_t i3 = radix_tree_get_index(page_number, 3);
    return &l3->children[i3];
}

struct span *page_map_get(size_t page_number) {
    void **span = page_map_get_ptr(page_number);

    if (span == NULL) {
        return NULL;
    }
    return *span;
}

static int page_map_insert_at(size_t page_number, struct span *span) {
    assert(page_number < MAX_NUM_PAGES);

    size_t i1 = radix_tree_get_index(page_number, 1);
    struct radix_tree_node *l2 = page_map->root.children[i1];
    if (l2 == NULL) {
        page_map->root.children[i1] = mmap_anonymous(sizeof(struct radix_tree_node));
        l2 = page_map->root.children[i1];
        if (l2 == NULL) {
            return -1;
        }
    }

    size_t i2 = radix_tree_get_index(page_number, 2);
    struct radix_tree_node *l3 = l2->children[i2];
    if (l3 == NULL) {
        l2->children[i2] = mmap_anonymous(sizeof(struct radix_tree_node));
        l3 = l2->children[i2];
        if (l3 == NULL) {
            munmap(l2, sizeof(struct radix_tree_node));
            return -1;
        }
    }

    size_t i3 = radix_tree_get_index(page_number, 3);
    l3->children[i3] = span;

    return 0;
}

int page_map_insert(struct span *span) {
    size_t page_number_start = (uintptr_t)span->page_start / PAGE_SIZE;
    size_t page_number_end = page_number_start + span->num_pages - 1;

    if (page_map_insert_at(page_number_start, span) == -1
        || page_map_insert_at(page_number_end, span) == -1
    ) {
        return -1;
    }
    return 0;
}

static void page_map_remove_at(size_t page_number) {
    void **span = page_map_get_ptr(page_number);
    if (span != NULL) {
        *span = NULL;
    }
}

void page_map_remove(struct span *span) {
    size_t page_number_start = (uintptr_t)span->page_start / PAGE_SIZE;
    size_t page_number_end = page_number_start + span->num_pages - 1;

    page_map_remove_at(page_number_start);
    page_map_remove_at(page_number_end);
}

static void radix_tree_node_print(struct radix_tree_node *node, size_t level, size_t page_number) {
    assert(1 <= level && level <= 3);
    assert(page_number < MAX_NUM_PAGES);

    if (level < 3) {
        for (size_t i = 0; i < RADIX_TREE_NODE_SIZE; i++) {
            if (node->children[i] == NULL) {
                continue;
            }
            radix_tree_node_print(
                node->children[i],
                level + 1,
                page_number | (i << (RADIX_TREE_NUM_BITS * (3 - level)))
            );
        }
    } else {
        for (size_t i = 0; i < RADIX_TREE_NODE_SIZE; i++) {
            struct span *span = node->children[i];
            if (span == NULL) {
                continue;
            }

            printf("  [page number 0x%zx] -> ", page_number | i);
            span_print(span);
            printf("\n");
        }
    }
}

void page_map_print() {
    printf("page_map\n");
    radix_tree_node_print(&page_map->root, 1, 0);
    printf("\n");
}
