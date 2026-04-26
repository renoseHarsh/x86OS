#include "elf.h"
#include "heap/heap.h"
#include "kprintf.h"
#include "layout.h"
#include "paging.h"
#include "pmm.h"
#include "process.h"
#include "sched.h"
#include "string.h"
#include "thread.h"
#include <stddef.h>
#include <stdint.h>

extern pde_t kernel_page_directory[];
extern void jump_usermode(uintptr_t eip);
extern size_t thread_id;

typedef struct {
    uint32_t kernel_stack_base;
    uint32_t kernel_esp;
} KERNEL_STACK_INFO;

KERNEL_STACK_INFO setup_kernel_stack(uintptr_t user_esp, uintptr_t entry)
{

    uint32_t *stack_base = kmalloc(0x1000);
    uint32_t *esp = &stack_base[1024];

    *--esp = (4 << 3 | 0 | 3); // SS
    *--esp = user_esp;
    *--esp = (1 << 9);         // eflags interrupt enabled
    *--esp = (3 << 3 | 0 | 3); // CS
    *--esp = entry;            // eip

    *--esp = 0; // error code
    *--esp = 0; // interrupt number

    for (int i = 0; i < 8; i++) {
        *--esp = 0;
    } // eax, ecx, edx, ebx, esp, ebp, esi, edi

    KERNEL_STACK_INFO info;
    info.kernel_stack_base = (uint32_t)stack_base;
    info.kernel_esp = (uint32_t)esp;

    return info;
}

Thread *create_process(void *elf_ptr)
{
    pde_t *process_pd = (pde_t *)P2V(pmm_alloc(0));
    kmemcpy(process_pd, kernel_page_directory, sizeof(pde_t) * 1024);

    uintptr_t stack_base = load_elf(process_pd, elf_ptr);

    if (!stack_base) {
        kprintf("Coundn't load\n");
        return NULL;
    }

    uint32_t *user_stack = (uintptr_t *)P2V(pmm_alloc(0));
    uintptr_t user_esp = stack_base + 0x1000;
    uintptr_t entry = ((Elf32_Ehdr *)elf_ptr)->e_entry;

    map_range(
        process_pd, V2P(user_stack), stack_base, 1,
        PAGE_PRESENT | PAGE_USER | PAGE_RW
    );

    KERNEL_STACK_INFO info = setup_kernel_stack(user_esp, entry);

    Thread *thread = kmalloc(sizeof(Thread));
    thread->que.next = NULL;
    thread->que.prev = NULL;
    thread->id = thread_id++;
    thread->status = RUNNABLE;
    thread->kernel_stack_base = info.kernel_stack_base;
    thread->kernel_esp = info.kernel_esp;
    thread->pd = process_pd;

    asm __volatile__("cli");
    sched_enqueue(thread);
    asm __volatile__("sti");

    return thread;
}

void destroy_process(Thread *thread)
{
    pde_t *pd = thread->pd;

    for (size_t pde_idx = 0; pde_idx < 768; pde_idx++) {
        if (pd[pde_idx] & PAGE_PRESENT) {
            uintptr_t pt = pd[pde_idx] & ~(0xFFF);
            pmm_free((void *)pt);
        }
    }
    pmm_free((void *)V2P(pd));
    kdestroy_thread(thread);
}
