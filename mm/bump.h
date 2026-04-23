#pragma once

#include <stddef.h>
#include <stdint.h>

void init_bump(uintptr_t end_addr);
void *bump_alloc(size_t num_pages);
uint32_t get_placement_addr();
