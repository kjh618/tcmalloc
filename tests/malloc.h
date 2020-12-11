#pragma once

#ifdef TEST_TC_MALLOC
#include "../tcmalloc/tcmalloc.h"
#define malloc(size) tc_malloc(size)
#define free(ptr) tc_free(ptr)
#endif

#ifdef TEST_LIBC_MALLOC
#include <stdlib.h>
#define tc_central_init()
#define tc_thread_init()
#endif
