# VGA Text Mode Driver

## Hardware Details
- **Memory Address:** `0xB8000`
- **Dimensions:** 80 columns x 25 rows
- **I/O Ports:**
  - `0x3D4`: Control Register (Index)
  - `0x3D5`: Data Register

## Color Byte Layout
Bit layout for the attribute byte (lower 8 bits of uint16_t):
| 7 | 6 5 4 | 3 | 2 1 0 |
|---|---|---|---|
| Blink | Background Color | Intensity | Foreground Color |

## I/O Port Commands
- **Cursor Positioning:**
    - Set Low Byte: Write `0x0F` to `0x3D4`, then write low byte to `0x3D5`
    - Set High Byte: Write `0x0E` to `0x3D4`, then write high byte to `0x3D5`

### References
- https://wiki.osdev.org/VGA_Text_Mode
- https://wiki.osdev.org/Text_UI
- https://wiki.osdev.org/Text_Mode_Cursor