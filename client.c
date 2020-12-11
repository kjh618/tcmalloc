#include "tcmalloc/tcmalloc.h"
#include <stdio.h>
#include <string.h>

struct stack {
    void *array[10000];
    size_t top;
};

struct stack stack;

static void stack_print() {
    printf("stack.array: ");
    for (size_t i = 0; i < stack.top; i++) {
        printf("%p, ", stack.array[i]);
    }
    printf("\n");
    printf("stack.top: %zu\n\n", stack.top);
}

static void stack_push(void *elem) {
    stack.array[stack.top] = elem;
    stack.top++;
}

static void *stack_pop() {
    stack.top--;
    return stack.array[stack.top];
}

int main(void) {
    tc_central_init();
    tc_thread_init();

    char func[10];
    while (1) {
        scanf("%s", func);

        size_t size, repeat;
        void *ptr;
        if (strcmp(func, "m") == 0) { // malloc
            scanf("%zu", &size);
            stack_push(tc_malloc(size));
        } else if (strcmp(func, "mr") == 0) { // malloc repeat
            scanf("%zu %zu", &size, &repeat);
            for (size_t i = 0; i < repeat; i++) {
                stack_push(tc_malloc(size));
            }
        } else if (strcmp(func, "f") == 0) { // free
            scanf("%p", &ptr);
            tc_free(ptr);
        } else if (strcmp(func, "fs") == 0) { // free stack
            tc_free(stack_pop());
        } else if (strcmp(func, "fsr") == 0) { // free stack repeat
            scanf("%zu", &repeat);
            for (size_t i = 0; i < repeat; i++) {
                tc_free(stack_pop());
            }
        } else if (strcmp(func, "d") == 0) { // debug print
            tc_debug_print();
            printf("\n");
        } else if (strcmp(func, "s") == 0) { // stack print
            stack_print();
            printf("\n");
        } else if (strcmp(func, "q") == 0) { // quit
            break;
        }
    }

    return 0;
}
