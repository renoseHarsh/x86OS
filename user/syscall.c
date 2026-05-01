#include "syscall.h"

void exit()
{
    __asm__ volatile("mov $0, %%eax\n"
                     "int $0x80\n" ::
                         : "eax");
}
