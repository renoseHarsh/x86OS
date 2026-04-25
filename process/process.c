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
extern void jump_usermode(uintptr_t eip);

void create_process(void *elf_ptr)
{
    pde_t *process_pd = (pde_t *)P2V(pmm_alloc(0));
    kmemcpy(process_pd, kernel_page_directory, sizeof(pde_t) * 1024);

    uintptr_t stack_base = load_elf(process_pd, elf_ptr);

    if (!stack_base) {
        kprintf("Coundn't load\n");
        return;
    }

    uint32_t *user_stack = (uintptr_t *)P2V(pmm_alloc(0));

    map_range(
        process_pd, V2P(user_stack), stack_base, 1,
        PAGE_PRESENT | PAGE_USER | PAGE_RW
    );

    uint32_t *sp = &user_stack[1024];
    uintptr_t user_esp = (uintptr_t)sp;

    *--sp = (uint32_t)(4 << 3 | 0 | 3);
    *--sp = stack_base + PAGE_SIZE;
    *--sp = (1 << 9);
    *--sp = (uint32_t)(3 << 3 | 0 | 3);
    *--sp = 0x40000000;

    refresh_cr3(process_pd);
    jump_usermode((uintptr_t)sp);
}
