#include "elf.h"
#include "layout.h"
#include "paging.h"
#include "pmm.h"
#include "string.h"
#include "utils.h"
#include <stddef.h>
#include <stdint.h>

unsigned char verify[4] = { 0x7f, 'E', 'L', 'F' };

void load_segment(
    size_t idx, Elf32_Phdr *phdrs, pde_t *pd, Elf32_Ehdr *elf_hdr
)
{
    Elf32_Phdr *cur = phdrs + idx;
    if (cur->p_type != PT_LOAD)
        return;

    uintptr_t vaddr = cur->p_vaddr;
    uintptr_t paddr = V2P((uintptr_t)elf_hdr + cur->p_offset);

    size_t filesz_pages = ALIGN_UP(cur->p_filesz, PAGE_SIZE) >> 12;
    size_t memsz_pages = ALIGN_UP(cur->p_memsz, PAGE_SIZE) >> 12;
    size_t zero_pages = memsz_pages - filesz_pages;

    uintptr_t zero_vaddr = vaddr + (filesz_pages << 12);

    uint32_t flags = PAGE_PRESENT | PAGE_USER;
    if (cur->p_flags == (PF_R | PF_W))
        flags |= PAGE_RW;

    if (filesz_pages)
        map_range(pd, paddr, vaddr, filesz_pages, flags);

    for (size_t i = 0; i < zero_pages; i++) {
        uintptr_t zero = (uintptr_t)pmm_alloc(0);
        map_range(pd, zero, zero_vaddr + (i << 12), 1, flags);
    }
}

bool load_elf(pde_t *pd, Elf32_Ehdr *elf_hdr)
{
    for (int i = 0; i < 4; i++) {
        if (elf_hdr->e_ident[i] != verify[i]) {
            return false;
        }
    }

    size_t header_n = elf_hdr->e_phnum;
    Elf32_Phdr *phdrs = (Elf32_Phdr *)((uintptr_t)elf_hdr + elf_hdr->e_phoff);

    for (size_t i = 0; i < header_n; i++) {
        load_segment(i, phdrs, pd, elf_hdr);
    }

    return true;
}
