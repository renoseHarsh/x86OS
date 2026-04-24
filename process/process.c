#include "elf.h"
#include "kprintf.h"
#include "layout.h"
#include "paging.h"
#include "pmm.h"
#include "process.h"
#include "string.h"
#include <stddef.h>
#include <stdint.h>

extern pde_t kernel_page_directory[];
extern void jump_usermode(uintptr_t eip, uintptr_t esp);

void create_process(void *elf_ptr)
{
    pde_t *process_pd = (pde_t *)P2V(pmm_alloc(0));
    kmemcpy(process_pd, kernel_page_directory, sizeof(pde_t) * 1024);
    if (!load_elf(process_pd, elf_ptr)) {
        kprintf("Coundn't load\n");
        return;
    }

    // uintptr_t page = (uintptr_t)pmm_alloc(0);
    // map_range(
    //     process_pd, page, 0x40000000, 1, PAGE_PRESENT | PAGE_RW | PAGE_USER
    // );
    // refresh_cr3(process_pd);
    // char *code = (char *)0x40000000;
    // code[0] = 0xCD; // int
    // code[1] = 0x80; // 0x80
    // code[2] = 0xEB; // jmp
    // code[3] = 0xFE; // $
    // uintptr_t user_stack = 0x40000000;
    // jump_usermode((uintptr_t)code, user_stack);
}
