#pragma once

#include "elf.h"
#include "thread.h"

Thread *create_process(Elf32_Ehdr *elf_hdr);
