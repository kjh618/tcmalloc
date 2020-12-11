#pragma once
#include <sys/types.h>
#include "object.h"

#define BATCH_SIZE 32

struct central_free_list;

extern struct central_free_list *central_free_list;

int central_free_list_new();

ssize_t central_free_list_get_objects(size_t class, struct object_list *free_objects, size_t batch_size);
void central_free_list_insert_object(size_t class, struct object *object);

void central_free_list_print();
