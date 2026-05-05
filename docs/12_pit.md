# System Timer: Programmable Interval Timer (PIT)

To implement preemptive scheduling, the kernel requires a hardware timer to periodically interrupt the CPU and invoke the scheduler. This is handled by the Intel 8253/8254 Programmable Interval Timer (PIT).

The PIT hardware ticks at a base frequency of exactly **1,193,181.8166 Hz** (often rounded to 1.193182 MHz), a value derived from the NTSC colorburst frequency for historical reasons. This base frequency is far too fast for context switching; if the scheduler ran at 1.19 MHz, the CPU would spend its entire execution time inside the context switch stub rather than running user code. To give each thread a usable ~10ms time slice, the PIT must be programmed to run at a much lower frequency (100 Hz).

## Frequency Scaling and the Divisor

The PIT scales its frequency using an internal 16-bit register called the **divisor**. On every pulse of the base clock, the divisor decrements. When it reaches zero, the PIT fires an interrupt on IRQ0 and resets the counter.

To calculate the correct divisor for a target frequency, we divide the base clock by the target clock:
*   **Formula:** `1193182 / 100 Hz` = `11931.82`
*   **Divisor:** `11932` (Rounded to the nearest 16-bit integer)
*   **Hexadecimal:** `0x2E9C`

**Note on Drift:** Because the hardware only accepts integer divisors, a perfect 100 Hz cannot be achieved. The actual frequency becomes `1193182 / 11932 ≈ 99.998 Hz`. This slight drift means each thread will run for a microscopic fraction longer than exactly 10ms, which is acceptable for general preemptive scheduling.

## Hardware Configuration

The PIT contains three channels. The kernel exclusively uses **Channel 0**, as it is the only channel hardwired to the Programmable Interrupt Controller (PIC) to fire `IRQ0`.

Communication with the PIT is done via two I/O ports:
1.  **Port `0x43` (Command Port):** Write-only. Used to set the operational mode and access parameters.
2.  **Port `0x40` (Data Port):** Read/Write. Used to set the 16-bit divisor value for Channel 0.

### The Command Byte
Before sending the divisor to Port `0x40`, the PIT must be configured via Port `0x43`. The kernel sends the command byte **`0x34`** (`00110100` in binary), which is constructed as follows:

*   **Bits 6-7 (`00`):** Select Channel 0.
*   **Bits 4-5 (`11`):** Access Mode (Lowbyte / Highbyte). This tells the PIT to expect the 16-bit divisor to be sent across the 8-bit data bus in two halves (lower 8 bits first, then upper 8 bits).
*   **Bits 1-3 (`010`):** Operating Mode 2 (Rate Generator).
*   **Bit 0 (`0`):** 16-bit binary mode (instead of BCD).

### Mode 2: Rate Generator
In **Rate Generator** mode, the PIT acts as a continuous divide-by-N counter. The output signal begins high and remains high as the internal counter decrements. 

The critical hardware timing occurs at the very end of the cycle: 
* When the internal count transitions from `2` to `1`, the output signal drops low for exactly one base clock cycle. 
* On the transition from `1` to `0`, the output immediately returns high, the `IRQ0` interrupt is fired, the original 16-bit divisor is automatically reloaded, and the cycle repeats.
