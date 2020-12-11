#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include "../tcmalloc/tcmalloc.h"
#include "vector.h"


/* Test 1 */

struct test1_malloc_info {
    void *ptr;
    size_t size;
};

static __thread size_t test1_num_mallocs;
static __thread struct test1_malloc_info test1_malloc_info[1000];

static size_t test1_malloc(size_t size) {
    size_t malloc_index = test1_num_mallocs;
    test1_malloc_info[malloc_index].ptr = tc_malloc(size);
    test1_malloc_info[malloc_index].size = size;
    test1_num_mallocs++;
    return malloc_index;
}

static unsigned char test1_generate_data(size_t tid, size_t malloc_index, size_t i) {
    return tid * 239 + malloc_index * 241 + i * 251;
}

static void test1_write_data(size_t tid, size_t malloc_index) {
    unsigned char *ptr = test1_malloc_info[malloc_index].ptr;
    size_t size = test1_malloc_info[malloc_index].size;
    assert(ptr != NULL && size > 0);

    for (size_t i = 0; i < size; i++) {
        ptr[i] = test1_generate_data(tid, malloc_index, i);
    }
}

static bool test1_check_data(size_t tid, size_t malloc_index) {
    unsigned char *ptr = test1_malloc_info[malloc_index].ptr;
    size_t size = test1_malloc_info[malloc_index].size;
    assert(ptr != NULL && size > 0);

    bool is_equal = true;
    for (size_t i = 0; i < size; i++) {
        unsigned char stored = ptr[i];
        unsigned char generated = test1_generate_data(tid, malloc_index, i);
        is_equal = is_equal && stored == generated;
    }

    return is_equal;
}

static void test1_free(size_t malloc_index) {
    assert(test1_malloc_info[malloc_index].ptr != NULL && test1_malloc_info[malloc_index].size > 0);

    tc_free(test1_malloc_info[malloc_index].ptr);
    test1_malloc_info[malloc_index].ptr = NULL;
    test1_malloc_info[malloc_index].size = 0;
}

static void *test1(void *ptid) {
    tc_thread_init();
    size_t tid = *(size_t *)ptid;

    size_t malloc_index = 0;

    for (size_t malloc_index = 0; malloc_index < 100; malloc_index++) {
        test1_malloc(8);
    }
    for (; malloc_index < 200; malloc_index++) {
        test1_malloc(128);
    }
    for (; malloc_index < 300; malloc_index++) {
        test1_malloc(1024);
    }
    for (; malloc_index < 310; malloc_index++) {
        test1_malloc(64 * 1024);
    }

    for (size_t malloc_index = 0; malloc_index < 310; malloc_index++) {
        test1_write_data(0, malloc_index);
    }

    // Check 1
    for (size_t malloc_index = 0; malloc_index < 310; malloc_index++) {
        if (test1_check_data(0, malloc_index) == false) {
            fprintf(stderr, "FAIL: tid: %zu, Check 1, malloc_index: %zu\n", tid, malloc_index);
            return NULL;
        }
    }

    for (size_t malloc_index = 100; malloc_index < 300; malloc_index++) {
        test1_free(malloc_index);
    }

    // Check 2
    for (size_t malloc_index = 0; malloc_index < 100; malloc_index++) {
        if (test1_check_data(0, malloc_index) == false) {
            fprintf(stderr, "FAIL: tid: %zu, Check 2, malloc_index: %zu\n", tid, malloc_index);
            return NULL;
        }
    }
    for (size_t malloc_index = 300; malloc_index < 310; malloc_index++) {
        if (test1_check_data(0, malloc_index) == false) {
            fprintf(stderr, "FAIL: tid: %zu, Check 2, malloc_index: %zu\n", tid, malloc_index);
            return NULL;
        }
    }

    for (size_t malloc_index = 300; malloc_index < 310; malloc_index++) {
        test1_free(malloc_index);
    }

    // Check 3
    for (size_t malloc_index = 0; malloc_index < 100; malloc_index++) {
        if (test1_check_data(0, malloc_index) == false) {
            fprintf(stderr, "FAIL: tid: %zu, Check 3, malloc_index: %zu\n", tid, malloc_index);
            return NULL;
        }
    }

    fprintf(stderr, "PASS: tid: %zu\n", tid);
    return NULL;
}


/* Test 2 */

#define INITIAL_CAPACITY 4
#define NUM_ROWS 10000
#define NUM_COLUMNS 1000

static size_t test2_generate_data(size_t i, size_t j) {
    return i * NUM_COLUMNS * 10 + j;
}

static void *test2(void *ptid) {
    tc_thread_init();
    size_t tid = *(size_t *)ptid;

    struct vector *vector_2d = vector_new(sizeof(struct vector *), true, INITIAL_CAPACITY);
    for (size_t i = 0; i < NUM_ROWS; i++) {
        struct vector *row = vector_new(sizeof(size_t), true, INITIAL_CAPACITY);
        vector_push(vector_2d, &row);
    }

    for (size_t i = 0; i < NUM_ROWS; i++) {
        struct vector *row = *(struct vector **)vector_ptr_at(vector_2d, i);
        for (size_t j = 0; j < NUM_COLUMNS; j++) {
            size_t data = test2_generate_data(i, j);
            vector_push(row, &data);
        }
    }

    // Check 1
    //tc_debug_print();
    for (size_t i = 0; i < NUM_ROWS; i++) {
        struct vector *row = *(struct vector **)vector_ptr_at(vector_2d, i);
        for (size_t j = 0; j < NUM_COLUMNS; j++) {
            size_t stored = *(size_t *)vector_ptr_at(row, j);
            size_t generated = test2_generate_data(i, j);
            if (stored != generated) {
                fprintf(stderr, "FAIL: tid: %zu, Check 1, [%zu][%zu], %zu != %zu\n", tid, i, j, stored, generated);
                return NULL;
            }
        }
    }

    size_t *array_2d[NUM_ROWS];
    for (size_t i = 0; i < NUM_ROWS; i++) {
        struct vector *row = *(struct vector **)vector_ptr_at(vector_2d, i);
        vector_shrink_to_fit(row);
        array_2d[i] = vector_into_array(row);
    }
    vector_free(vector_2d, NULL);

    // Check 2
    //tc_debug_print();
    for (size_t i = 0; i < NUM_ROWS; i++) {
        for (size_t j = 0; j < NUM_COLUMNS; j++) {
            size_t stored = array_2d[i][j];
            size_t generated = test2_generate_data(i, j);
            if (stored != generated) {
                fprintf(stderr, "FAIL: tid: %zu, Check 2, [%zu][%zu], %zu != %zu\n", tid, i, j, stored, generated);
                return NULL;
            }
        }
    }

    for (size_t i = 0; i < NUM_ROWS; i++) {
        tc_free(array_2d[i]);
    }

    fprintf(stderr, "PASS: tid: %zu\n", tid);
    return NULL;
}


/* Main */

typedef void *test_func(void *ptid);
static test_func *const TESTS[] = { NULL, test1, test2 };

static void test_st(size_t test_num) {
    size_t tid = 0;
    TESTS[test_num](&tid);
}

static void test_mt(size_t test_num, size_t num_threads) {
    size_t tids[num_threads];
    pthread_t threads[num_threads];

    for (size_t i = 0; i < num_threads; i++) {
        tids[i] = i;
        if (pthread_create(&threads[i], NULL, TESTS[test_num], &tids[i]) != 0) {
            perror("pthread_create");
            return;
        }
    }

    for (size_t i = 0; i < num_threads; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("pthread_join");
            return;
        }
    }
}

int main(int argc, char *argv[]) {
    tc_central_init();

    if (argc >= 2) {
        if (strcmp(argv[1], "st") == 0 && argc == 3) {
            size_t test_num = atoi(argv[2]);
            test_st(test_num);
            return 0;
        } else if (strcmp(argv[1], "mt") == 0 && argc == 4) {
            size_t test_num = atoi(argv[2]);
            size_t num_threads = atoi(argv[3]);
            test_mt(test_num, num_threads);
            return 0;
        }
    }

    fprintf(stderr, "Wrong argument format\n");
    return -1;
}
