#include "elf.h"
#include "heap/heap.h"
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
extern void launch_process();
extern size_t thread_id;

typedef struct {
    void *kernel_stack_base;
    uintptr_t kernel_esp;
} KStackInfo;

KStackInfo setup_kerenl_stack(uintptr_t entry, uintptr_t user_esp)
{
    void *stack = kmalloc(0x1000);
    uint32_t *esp = stack + 0x1000;

    *--esp = (4 << 3 | 0 | 3); // ss
    *--esp = user_esp;         // esp
    *--esp = (1 << 9);         // eflags IF set
    *--esp = (3 << 3 | 0 | 3); // cs
    *--esp = entry;            // eip

    *--esp = (uintptr_t)launch_process;

    *--esp = 0; // eax
    *--esp = 0; // ecx
    *--esp = 0; // edx
    *--esp = 0; // ebx
    *--esp = 0; // esp (ignored by popa)
    *--esp = 0; // ebp
    *--esp = 0; // esi
    *--esp = 0; // edi

    KStackInfo info;
    info.kernel_esp = (uintptr_t)esp;
    info.kernel_stack_base = stack;

    return info;
}

Thread *create_process(Elf32_Ehdr *elf_hdr)
{
    pde_t *process_pd = (pde_t *)P2V(pmm_alloc(0));
    kmemcpy(process_pd, kernel_page_directory, 0x1000);

    uintptr_t stack_base = load_elf(process_pd, elf_hdr);

    uintptr_t user_stack = (uintptr_t)pmm_alloc(0);
    uintptr_t user_esp = stack_base + 0x1000;
    uintptr_t entry = elf_hdr->e_entry;

    map_range(
        process_pd, user_stack, stack_base, 1,
        PAGE_USER | PAGE_PRESENT | PAGE_RW
    );

    KStackInfo stack_info = setup_kerenl_stack(entry, user_esp);

    Thread *thread = kmalloc(sizeof(Thread));

    thread->esp = stack_info.kernel_esp;
    thread->stack = stack_info.kernel_stack_base;
    thread->id = thread_id++;
    thread->pd = process_pd;

    thread_ready(thread);

    return thread;
}
