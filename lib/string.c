#include "string.h"
#include <stddef.h>
#include <stdint.h>

void *kmemcpy(void *dest, const void *src, size_t count)
{
    uint32_t *ptr = (uint32_t *)dest;
    uint32_t *src_ptr = (uint32_t *)src;
    size_t dwords = count / 4;
    size_t bytes = count % 4;

    __asm__ volatile("cld\n\t"
                     "rep movsl\n\t"
                     "mov %3, %%ecx\n\t"
                     "rep movsb\n\t"
                     : "+D"(ptr), "+S"(src_ptr), "+c"(dwords)
                     : "r"(bytes)
                     : "memory");

    return dest;
}

void *kmemset(void *dest, uint8_t value, size_t count)
{
    uint32_t *ptr = (uint32_t *)dest;
    size_t dwords = count / 4;
    size_t bytes = count % 4;
    uint32_t val32 = (uint32_t)value;
    val32 |= val32 << 8;
    val32 |= val32 << 16;

    __asm__ volatile("cld\n\t"
                     "rep stosl\n\t"
                     "mov %3, %%ecx\n\t"
                     "rep stosb\n\t"
                     : "+D"(ptr), "+c"(dwords)
                     : "a"(val32), "r"(bytes)
                     : "memory");
    return dest;
}

void *kwmemset(void *dest, uint16_t value, size_t count)
{
    uint32_t *ptr = (uint32_t *)dest;
    size_t dwords = count / 2;
    size_t bytes = count % 2;
    uint32_t val32 = (uint32_t)value;
    val32 |= val32 << 16;

    __asm__ volatile("cld\n\t"
                     "rep stosl\n\t"
                     "mov %3, %%ecx\n\t"
                     "rep stosw\n\t"
                     : "+D"(ptr), "+c"(dwords)
                     : "a"(val32), "r"(bytes)
                     : "memory");
    return dest;
}
