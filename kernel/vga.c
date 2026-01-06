#include "ports.h"
#include "string.h"
#include "vga.h"
#include <stdint.h>

// VGA text mode buffer starts at physical address 0xB8000
#define VGA_MEMORY (uint16_t *)0xB8000
// VGA text mode dimensions
#define columns 80
#define rows 25

static uint8_t vga_color = 0x0F; // Default: white on black
static uint16_t *const vga_buffer = VGA_MEMORY;
static uint8_t cursor_row = 2;
static uint8_t cursor_col = 0;

static void vga_move_cursor()
{
    uint16_t pos = cursor_row * columns + cursor_col;

    // Send the high byte and low byte to the VGA controller
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

static void vga_scroll()
{
    // Move all rows up by one
    for (size_t r = 1; r < rows; r++) {
        kmemcpy(
            vga_buffer + (80 * (r - 1)), vga_buffer + (80 * r),
            columns * sizeof(uint16_t)
        );
    }
    // Clear the last row
    kwmemset(vga_buffer + (80 * (rows - 1)), ' ' | (vga_color << 8), columns);
}

void vga_clear()
{
    // Reset cursor position
    cursor_row = 0;
    cursor_col = 0;
    vga_move_cursor();
    // Clear the screen
    kwmemset(vga_buffer, ' ' | (vga_color << 8), columns * rows);
}

void vga_set_color(const enum vga_color fg, const enum vga_color bg)
{
    vga_color = (bg << 4 | fg);
}

void vga_putc(const char c)
{
    if (cursor_row == rows) {
        vga_scroll();
        cursor_row = rows - 1;
    }
    if (c == '\n') {
        cursor_row++;
        cursor_col = 0;
    } else {
        vga_buffer[(cursor_row * columns) + cursor_col++]
            = c | (vga_color << 8);
        if (cursor_col == columns) {
            cursor_row++;
            cursor_col = 0;
        }
    }
    vga_move_cursor();
}

void vga_puts(const char *str)
{
    for (; *str != '\0'; str++)
        vga_putc(*str);
}
