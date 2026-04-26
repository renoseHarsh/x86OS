#include "syscall.h"

void exit()
{
    __asm__ volatile("mov $1, %%eax\n"
                     "int $0x80\n" ::
                         : "eax");
}
