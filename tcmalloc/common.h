#pragma once
#include <stddef.h>
#include <pthread.h>

#define PAGE_SIZE 4096
#define MAX_NUM_PAGES ((size_t)1 << 52)

/* Size classes:
 * 8, 16, ..., 64, (8)
 * 128, 192, ..., 2048, (31)
 * 2304, 2560, ..., 32768 (120)
 */
#define SMALL_OBJECT_SIZE_LIMIT 32768
#define NUM_CLASSES 159

extern pthread_spinlock_t global_lock;

size_t class_from_size(size_t size);
size_t class_get_size(size_t class);

void *mmap_anonymous(size_t size);
