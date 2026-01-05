# ATA PIO Driver

The ATA PIO (Programmed Input/Output) mode is a method used by computers to communicate with storage devices such as hard drives and optical drives. In PIO mode, data is transferred between the CPU and the storage device using the system's main memory and CPU cycles, rather than using direct memory access (DMA).

### Why ATA PIO over BIOS Int 13h?
In real mode I cannot access the 0x100000 (1MiB) memory area where I want my kernel to be loaded. Therefore I cannot use BIOS interrupts like Int 13h to read from the disk. Instead, I need to implement my own ATA PIO routines to read the kernel from the disk into memory.

#### ATA PIO Basics
ATA devices are typically accessed through I/O ports. The primary and secondary ATA channels use specific I/O port ranges:
- Primary Channel: 0x1F0 to 0x1F7
- Secondary Channel: 0x170 to 0x177

(For this project, I only used the primary channel.)

### ATA PIO Read Sequence
1. **Select the Drive**: Write to the drive/head register (0x1F6) to select the master or slave drive, set the LBA mode and pass the high 4 bits of the LBA address.
2. **Set the Sector Count**: Write the number of sectors to read (0 means 256 secotrs) to the sector count register (0x1F2).
3. **Set the LBA Address**: Write the LBA address to the sector number (0x1F3), cylinder low (0x1F4), and cylinder high (0x1F5) registers.
4. **Issue the Read Command**: Write the read command (0x20 for read sectors) to the command register (0x1F7).
5. **Wait for Data Ready**: Poll the status register (0x1F7) until the DRQ (Data Request) bit is set, indicating that the data is ready to be read.
6. **Read the Data**: Read the data from the data register (0x1F0) in 16-bit words. Each sector is 512 bytes, so you need to read 256 words per sector. 

### Problems Encountered
If I read more sectors than available on the disk, the drive will not set the DRQ bit, causing an infinite wait in the read loop. To avoid this, ensure that the number of sectors requested does not exceed the total sectors available on the disk. To Detect the error we can read the error bit from the status register (0x1F7) after issuing the read command. If an error occurs, the ERR bit will be set, and we can handle it accordingly.

### Footnote
I used the lba 28 bit mode this limits the access to disks up to 128 GiB which is sufficient for this project.

#### References
- https://wiki.osdev.org/ATA_PIO_Mode