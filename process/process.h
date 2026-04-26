#pragma once

#include "thread.h"
#include <stdint.h>

Thread *create_process(void *elf_ptr);
void destroy_process(Thread *thread);
