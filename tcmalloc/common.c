#include "common.h"
#include <sys/mman.h>
#include <assert.h>

pthread_spinlock_t global_lock;

size_t class_from_size(size_t size) {
    assert(1 <= size && size <= SMALL_OBJECT_SIZE_LIMIT);

    if (size <= 64) {
        return (size - 1) / 8;
    } else if (size <= 2048) {
        return 8 + (size - 64 - 1) / 64;
    } else {
        return 8 + 31 + (size - 2048 - 1) / 256;
    }
}

size_t class_get_size(size_t class) {
    assert(class < NUM_CLASSES);

    if (class < 8) {
        return 8 + class * 8;
    } else if (class < 8 + 31) {
        return 128 + (class - 8) * 64;
    } else {
        return 2304 + (class - 8 - 31) * 256;
    }
}

void *mmap_anonymous(size_t size) {
    return mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
}
