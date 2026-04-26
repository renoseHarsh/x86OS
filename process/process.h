#pragma once

#include "thread.h"
#include <stdint.h>

void create_process(void *elf_ptr);
void destroy_process(Thread *thread);
