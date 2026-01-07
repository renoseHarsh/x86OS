#include "vga.h"
#include <stdarg.h>
#include <stdbool.h>

static int print_unsigned(unsigned int n, int base)
{
    const char *digits = "0123456789ABCDEF";
    char buf[32];
    int i = 0;

    if (n == 0) {
        vga_putc('0');
        return 1;
    }

    while (n > 0) {
        buf[i++] = digits[n % base];
        n /= base;
    }

    int count = 0;
    while (i--) {
        count++;
        vga_putc(buf[i]);
    }
    return count;
}

static int print_signed(int n)
{
    if (n < 0) {
        int count = 1;
        vga_putc('-');
        return count + print_unsigned((unsigned int)(-n), 10);
    }
    return print_unsigned((unsigned int)n, 10);
}

static int print_str(const char *str)
{
    int count = 0;
    while (*str != '\0') {
        vga_putc(*str++);
        count++;
    }
    return count;
}

int kprintf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    int count = 0;

    while (*fmt != '\0') {
        if (*fmt == '%') {
            switch (*(++fmt)) {
            case 'd': {
                count += print_signed(va_arg(args, int));
                break;
            }
            case 'u': {
                count += print_unsigned(va_arg(args, unsigned int), 10);
                break;
            }
            case 'x': {
                count += print_unsigned(va_arg(args, int), 16);
                break;
            }
            case 'c': {
                count++;
                vga_putc((char)va_arg(args, int));
                break;
            }
            case 's': {
                count += print_str(va_arg(args, char *));
                break;
            }
            case '%': {
                vga_putc('%');
                count++;
                break;
            }
            default: {
                vga_putc('%');
                vga_putc(*fmt);
                count += 2; // for '%' and the character
                break;
            }
            }
        } else {
            vga_putc(*fmt);
            count++;
        }
        fmt++;
    }

    va_end(args);
    return count;
}
