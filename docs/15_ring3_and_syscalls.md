# Ring 3 & System Calls

To safely execute user-space programs, the kernel must drop its privileges and transition the CPU into **Ring 3**. This hardware-enforced isolation prevents user applications from executing privileged instructions (like `cli` or `hlt`), modifying page tables, or directly accessing kernel memory. 

Once in Ring 3, the user program requires a secure gateway to request services from the kernel, which is handled via System Calls.

## 1. Getting to Ring 3 (The Privilege Drop)

Modern x86 architectures offer multiple instructions for privilege transitions (`sysret`, `sysexit`, `iret`). 
*   `sysret` is exclusive to 64-bit Long Mode.
*   `sysexit` is fast but requires manual stack management by the OS upon kernel entry.

For architectural simplicity and consistency with standard hardware interrupts, this kernel utilizes the **`iret` (Interrupt Return)** instruction to drop privileges. Because x86 does not possess a direct "go to Ring 3" instruction, the kernel must fake a return from a Ring 0 interrupt.

### The Forged `iret` Stack
To trick the CPU into dropping privileges, the kernel hand-forges an interrupt frame on the current Ring 0 stack. When `iret` is executed, the CPU pops these 5 values to restore the state:

| Stack Order (Top to Bottom) | Register | Value / Purpose |
| :--- | :--- | :--- |
| `ESP + 16` | **SS** (Stack Segment) | User Data Segment selector (with RPL = 3). |
| `ESP + 12` | **ESP** (Stack Pointer) | The allocated Ring 3 stack pointer for the user program. |
| `ESP + 08` | **EFLAGS** | `0x202` (Interrupts Enabled `IF` set, so the PIT can preempt). |
| `ESP + 04` | **CS** (Code Segment) | User Code Segment selector (with RPL = 3). |
| `ESP + 00` | **EIP** (Instr. Pointer) | The entry point parsed from the user ELF binary. |

**Segment Register Caveat:** While `iret` automatically restores `CS` and `SS`, it does *not* touch the general data segments (`DS`, `ES`, `FS`, `GS`). Before executing the `iret` stub, the kernel must manually update these registers to point to the User Data Segment. Consequently, whenever a hardware interrupt preempts a Ring 3 task, the kernel's interrupt handlers must save these user segments and reload the Ring 0 kernel segments to function safely.

## 2. The Task State Segment (TSS)

When a user program is executing in Ring 3 and a hardware interrupt fires (or a syscall is invoked), the CPU must immediately switch back to Ring 0. 

However, the CPU cannot push the Ring 3 state onto the Ring 3 stack, as that stack cannot be trusted (the user might have trashed the stack pointer). The hardware needs a known, safe Ring 0 stack to use during the transition. 

This is provided by the **Task State Segment (TSS)**. The kernel maintains a single global TSS entry. We only populate one critical field: `TSS.ESP0`. Before jumping to Ring 3, the kernel writes the top of the thread's Ring 0 stack into `TSS.ESP0`. When the CPU transitions from Ring 3 to Ring 0, it automatically fetches this pointer and safely pushes the `SS, ESP, EFLAGS, CS, EIP` frame onto the kernel stack.

## 3. The System Call Interface (`int 0x80`)

Because Ring 3 programs are isolated, they must invoke the kernel to perform operations like writing to the screen or terminating. This is achieved via a software interrupt.

The kernel registers Interrupt `0x80` in the IDT with a Descriptor Privilege Level (DPL) of 3, allowing user-space code to trigger it without causing a General Protection Fault. 

*(Note: While `sysenter` is technically faster than `int 0x80`, it bypasses the TSS hardware stack-swap. Using `int 0x80` leverages the CPU's native interrupt handling to safely manage the Ring 3 to Ring 0 stack transition).*

### The Application Binary Interface (ABI)
Data is passed between Ring 3 and Ring 0 strictly through CPU registers. 

1.  **Syscall Number:** The user program places the desired system call ID into `EAX` (e.g., `1` for exit, `4` for write).
2.  **Arguments:** Parameters are placed in `EBX`, `ECX`, `EDX`, etc.
3.  **Invocation:** The program executes `int 0x80`.
4.  **Return Value:** Upon completion, the kernel places the result back into `EAX` before executing `iret` to resume the user program.

### The Dispatcher
When `int 0x80` is triggered, the kernel's common interrupt handler saves the user registers and passes them as a struct to the **Syscall Dispatcher**. The dispatcher uses a `switch` statement mapped against the `EAX` register to route the execution flow to the appropriate kernel function (e.g., `sys_write`, `sys_exit`).

## References
*   [OSDev: Getting to Ring 3](https://wiki.osdev.org/Getting_to_Ring_3)
*   [OSDev: System Calls](http://wiki.osdev.org/System_Calls)
