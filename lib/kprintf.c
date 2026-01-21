#include "vga.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

static const char *digits = "0123456789ABCDEF";

static int print_unsigned(uint64_t n, int base)
{
    char buf[64];
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

static int print_signed(int64_t n)
{
    if (n < 0) {
        int count = 1;
        vga_putc('-');
        return count + print_unsigned((uint64_t)(-n), 10);
    }
    return print_unsigned((uint64_t)n, 10);
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

            bool long_flag = 0;
            bool long_check = 0;
            if (*(++fmt) == 'l') {
                long_check = 1;
                if (*(++fmt) == 'l') {
                    long_flag = true;
                    fmt++;
                }
            }

            switch (*fmt) {
            case 'd': {
                if (long_flag)
                    count += print_signed(va_arg(args, int64_t));
                else
                    count += print_signed((int32_t)va_arg(args, int32_t));
                break;
            }
            case 'u': {
                if (long_flag)
                    count += print_unsigned(va_arg(args, uint64_t), 10);
                else
                    count += print_unsigned(
                        (uint32_t)va_arg(args, uint32_t), 10
                    );
                break;
            }
            case 'x': {
                if (long_flag)
                    count += print_unsigned(va_arg(args, uint64_t), 16);
                else
                    count += print_unsigned(
                        (uint32_t)va_arg(args, uint32_t), 16
                    );
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
                if (long_check) {
                    vga_putc('l');
                    count++;
                }
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
