CC = gcc
CCFLAGS = -g -Wall -Wextra -pthread

%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@

all: tests/tests client
clean:
	rm -fv tcmalloc/*.o tests/*.o tests/tests
	rm -fv *.o client

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

tests/tests: tests/tests.o tests/vector.o $(TCMALLOC_O)
	$(CC) $(CCFLAGS) $^ -o $@
tests/tests.o: tcmalloc/tcmalloc.h
tests/vector.o: tests/vector.h tcmalloc/tcmalloc.h

client: client.o $(TCMALLOC_O)
	$(CC) $(CCFLAGS) $^ -o $@
client.o: tcmalloc/tcmalloc.h
