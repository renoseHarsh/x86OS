global _entry
extern main
extern exit

section .text
_entry:
    call main
    call exit
