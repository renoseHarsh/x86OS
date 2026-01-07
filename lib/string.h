#pragma once
#include <stddef.h>
#include <stdint.h>

void *kmemcpy(void *dest, const void *src, size_t count);
void *kwmemset(void *dest, uint16_t value, size_t count);
void *kmemset(void *dest, uint8_t value, size_t count);
