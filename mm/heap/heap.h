#pragma once

#include <stddef.h>
void init_kmalloc();
void *kmalloc(size_t size);
void kfree(void *ptr);
