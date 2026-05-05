# X86OS: A 32-bit x86

A custom 32-bit Unix-like operating system targeting the x86 architecture. Built entirely from scratch in C and Assembly, X86OS is Multiboot2 compliant, boots via GRUB, and features preemptive multitasking, a custom physical memory manager, and user-space ELF execution.

## Core Features

*   **Memory Management:** Higher-half kernel mapping, Buddy System allocator for physical pages, segregated linked-list kernel heap, and isolated per-process page directories.
*   **Process & Scheduling:** Preemptive Round-Robin thread scheduling, full thread lifecycle management including a dedicated zombie reaper thread, and hardware context switching via TSS.
*   **User Space & Execution:** Dynamic ELF binary loading from Multiboot2 modules, `iret`-based transitions to Ring 3, and a standard system call ABI.
*   **Hardware & Drivers:** 8259 PIC, 8253 PIT, VGA text mode, and Serial port output for kernel debugging.

## Architecture & Design Decisions

Detailed architectural breakdowns can be found in the [`docs/`](docs/) directory. Below is an executive summary of core subsystem designs:

### Physical Memory Manager (PMM) & Heap
Instead of a standard bitmap, the PMM utilizes an array-based memory map to track every 4KiB page. This design prevents cache thrashing when traversing memory and stores crucial metadata (e.g., chunk heads and sizes directly in the array items). This allows the kernel to perform `free()` operations without explicitly passing allocation sizes. The kernel heap currently utilizes a segregated linked list to support variable-sized object allocations, prioritizing flexibility before transitioning high-frequency objects to a slab allocator. 
*Read more in [10_physical_memory_manager.md](docs/10_physical_memory_manager.md) and [11_kernel_heap.md](docs/11_kernel_heap.md).*

### Scheduler & Thread Lifecycle
Threads are initialized with a simulated stack frame. Kernel threads execute their target function and, upon return, are pushed to a zombie queue for safe memory reclamation by a background reaper thread. User processes are bootstrapped via a kernel stub that executes an `iret` to transition to Ring 3, where a `crt0` wrapper invokes `main` and handles the `sys_exit` cleanup. Because the kernel currently targets single-core execution, scheduler queues and shared variables are protected atomically via interrupt masking (`cli`/`sti`). 
*Read more in [16_v2_scheduler_and_reaper.md](docs/16_v2_scheduler_and_reaper.md).*


## Project Layout

```text
.
├── docs/       # Extensive subsystem design documentation
├── drivers/    # Hardware interaction (PIC, PIT, VGA, Serial)
├── gdt/        # Global Descriptor Table and Ring 3 segments
├── interrupts/ # IDT, ISRs, and exception handling
├── kernel/     # Core initialization, ports, and reaper thread
├── lib/        # Kernel-level standard library (kprintf, strings, panic)
├── mm/         # Buddy allocator, Paging, and Kernel Heap
├── process/    # ELF loading, Syscalls, and user-space transitions
├── sched/      # Round-Robin scheduler, queues, and context switching
└── user/       # Ring 3 userland library stubs and init processes
```

## Building and Running

**Target Architecture:** x86 (32-bit)  
**Supported Emulator:** QEMU

### Dependencies
*   `i686-elf-gcc` (Cross-compiler)
*   `make`
*   `grub-mkrescue` & `xorriso`
*   `qemu-system-i386`


## Commands

### Build the OS ISO:

```bash
make
```

### Run in QEMU:

```bash
make run
```

### Debug Mode:

*(Boots QEMU normally and automatically attaches a live GDB session)*

```bash
make debug
```

## Future Roadmap

*   **Expanded Syscall Interface:** Implementing a broader set of Unix-like system calls to support more complex userland applications.
*   **Virtual File System (VFS):** Abstracting storage to support file operations and standard file descriptors.
*   **Slab Allocator:** Optimizing the kernel heap for high-frequency, fixed-size kernel object allocations.
