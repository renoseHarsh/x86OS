global kernel_entry
extern kmain

kernel_entry:
    call kmain
    .hang:
        cli
        hlt
        jmp .hang
