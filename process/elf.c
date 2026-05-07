#include "elf.h"
#include "layout.h"
#include "paging.h"
#include "pmm.h"
#include "string.h"
#include "utils.h"
#include <stddef.h>
#include <stdint.h>

unsigned char verify[4] = { 0x7f, 'E', 'L', 'F' };

uintptr_t load_segment(
    size_t idx, Elf32_Phdr *phdrs, pde_t *pd, Elf32_Ehdr *elf_hdr
)
{
    Elf32_Phdr *cur = phdrs + idx;
    if (cur->p_type != PT_LOAD)
        return 0x0;

    uint32_t flags = PAGE_PRESENT | PAGE_USER;
    if (cur->p_flags == (PF_R | PF_W))
        flags |= PAGE_RW;

    uintptr_t vaddr = cur->p_vaddr;
    uint8_t *elf_src_ptr = ((uint8_t *)elf_hdr) + cur->p_offset;

    size_t pages_needed = ALIGN_UP(cur->p_memsz, PAGE_SIZE) >> 12;

    size_t copy_remaining = cur->p_filesz;

    for (size_t i = 0; i < pages_needed; i++) {
        void *cur_page = pmm_alloc(0);
        void *kernel_vir_cur_page = (void *)P2V(cur_page);

        kmemset(kernel_vir_cur_page, 0, PAGE_SIZE);
        size_t copying = copy_remaining;
        if (copy_remaining) {
            copying
                = (copy_remaining > PAGE_SIZE) ? PAGE_SIZE : copy_remaining;
            kmemcpy(kernel_vir_cur_page, elf_src_ptr + (i << 12), copying);
            copy_remaining -= copying;
        }
        map_range(pd, (uintptr_t)cur_page, vaddr + (i << 12), 1, flags);
    };

    return cur->p_vaddr + (pages_needed << 12);
}

uintptr_t load_elf(pde_t *pd, Elf32_Ehdr *elf_hdr)
{
    for (int i = 0; i < 4; i++) {
        if (elf_hdr->e_ident[i] != verify[i]) {
            return 0x0;
        }
    }

    size_t header_n = elf_hdr->e_phnum;
    Elf32_Phdr *phdrs = (Elf32_Phdr *)((uintptr_t)elf_hdr + elf_hdr->e_phoff);

    uintptr_t last_addr = 0;

    for (size_t i = 0; i < header_n; i++) {
        uintptr_t ends_at = load_segment(i, phdrs, pd, elf_hdr);
        if (ends_at > last_addr)
            last_addr = ends_at;
    }

    return last_addr;
}
