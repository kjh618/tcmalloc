CC = gcc
CCFLAGS = -g -Wall -Wextra -pthread

%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@

all: tests/tests_libc tests/tests_tc client
clean:
	rm -fv tcmalloc/*.o
	rm -fv tests/*.o tests/tests_libc tests/tests_tc
	rm -fv *.o client

# TCMalloc
tcmalloc/tcmalloc.o: tcmalloc/tcmalloc.h tcmalloc/list.h tcmalloc/common.h tcmalloc/object.h tcmalloc/span.h tcmalloc/page_map.h tcmalloc/page_heap.h tcmalloc/central_free_list.h tcmalloc/thread_cache.h
tcmalloc/list.o: tcmalloc/list.h
tcmalloc/common.o: tcmalloc/common.h
tcmalloc/object.o: tcmalloc/object.h
tcmalloc/span.o: tcmalloc/span.h tcmalloc/list.h tcmalloc/common.h tcmalloc/object.h tcmalloc/page_map.h
tcmalloc/page_map.o: tcmalloc/page_map.h tcmalloc/common.h tcmalloc/span.h
tcmalloc/page_heap.o: tcmalloc/page_heap.h tcmalloc/common.h
tcmalloc/central_free_list.o: tcmalloc/central_free_list.h tcmalloc/list.h tcmalloc/common.h tcmalloc/span.h tcmalloc/page_heap.h
tcmalloc/thread_cache.o: tcmalloc/thread_cache.h tcmalloc/common.h tcmalloc/object.h tcmalloc/central_free_list.h
TCMALLOC_O = tcmalloc/tcmalloc.o tcmalloc/list.o tcmalloc/common.o tcmalloc/object.o tcmalloc/span.o tcmalloc/page_map.o tcmalloc/page_heap.o tcmalloc/central_free_list.o tcmalloc/thread_cache.o

# Tests with libc malloc
tests/tests_libc: tests/tests_libc.o tests/vector_libc.o
	$(CC) $(CCFLAGS) $^ -o $@
tests/tests_libc.o: tests/tests.c tests/malloc.h tests/vector.h
	$(CC) $(CCFLAGS) -D TEST_LIBC_MALLOC -c $< -o $@
tests/vector_libc.o: tests/vector.c tests/vector.h tests/malloc.h
	$(CC) $(CCFLAGS) -D TEST_LIBC_MALLOC -c $< -o $@

# Tests with TCMalloc
tests/tests_tc: tests/tests_tc.o tests/vector_tc.o $(TCMALLOC_O)
	$(CC) $(CCFLAGS) $^ -o $@
tests/tests_tc.o: tests/tests.c tests/malloc.h tests/vector.h
	$(CC) $(CCFLAGS) -D TEST_TC_MALLOC -c $< -o $@
tests/vector_tc.o: tests/vector.c tests/vector.h tests/malloc.h
	$(CC) $(CCFLAGS) -D TEST_TC_MALLOC -c $< -o $@

# Client
client: client.o $(TCMALLOC_O)
	$(CC) $(CCFLAGS) $^ -o $@
client.o: tcmalloc/tcmalloc.h
