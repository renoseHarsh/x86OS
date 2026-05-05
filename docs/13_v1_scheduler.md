# Kernel Thread Scheduler (v1)

The v1 scheduler is responsible for multiplexing execution across multiple Ring 0 kernel threads. It separates the **policy** (picking the next thread via a queue) from the **mechanism** (the low-level assembly context switch).

## 1. The Context Switch Mechanism

To switch execution from one thread to another, the CPU must save the state of the outgoing thread and restore the state of the incoming thread. Instead of manually saving registers into the Thread Control Block (TCB), the kernel leverages the execution stack itself.

The core mechanism is driven by an assembly stub:
```c
extern void context_switch(uint32_t **old_esp, uint32_t *new_esp);
```

### Execution Flow:
1.  **Save State:** When `context_switch` is called, the C compiler pushes the arguments, and the `call` instruction pushes the current `EIP` (Instruction Pointer) onto the stack. Inside the stub, `pusha` is executed to push all 8 general-purpose registers onto the stack.
2.  **Swap Stacks:** The current `ESP` is saved into the memory location pointed to by `old_esp` (inside the outgoing thread's TCB). Then, `ESP` is overwritten with the value of `new_esp` (from the incoming thread's TCB).
3.  **Restore State:** The CPU is now operating on the incoming thread's stack. The stub executes `popa`, which consumes the top 8 values on this new stack and restores the general-purpose registers.
4.  **Resume Execution:** Finally, the `ret` instruction is executed. It pops the next 4-byte value off the stack into the `EIP` and jumps to it, seamlessly resuming the incoming thread.

*Note: This stub must be written in pure assembly to guarantee exact control over the stack alignment and instruction order.*

## 2. Thread Initialization (Stack Forging)

For the `context_switch` stub to work, a newly created thread must look exactly like a thread that was previously suspended by `context_switch`. 

When a new kernel thread is allocated, the kernel requests a fresh page for its stack and manually "forges" the stack layout from the top down.

### The Forged Stack Layout
To conform to the standard C calling convention (`cdecl`) and prepare for the `popa` / `ret` sequence, the initial stack is structured as follows:

| Offset (Top to Bottom) | Value | Purpose |
| :--- | :--- | :--- |
| `ESP + 00` to `+ 28` | `0x00000000` (x8) | 8 dummy zero values consumed by the `popa` instruction. |
| `ESP + 32` | `&kthread_stub` | Consumed by the `ret` instruction to jump to the stub. |
| `ESP + 36` | `0x00000000` | Fake return address for `kthread_stub` (never used). |
| `ESP + 40` | `void (*entry)(void *)` | Argument 1: The thread's actual payload function. |
| `ESP + 44` | `void *arg` | Argument 2: The parameter to pass to the payload. |
| `ESP + 48` | `Thread *thread` | Argument 3: Pointer to the thread's own TCB. |

## 3. The Thread Execution Stub

Instead of jumping directly into the user-provided entry function, the forged `EIP` points to a wrapper function.

```c
static void kthread_stub(void (*entry)(void *), void *arg, Thread *thread);
```

**Responsibilities:**
1.  **Execution:** It invokes the actual payload function passed in via the forged stack arguments.
2.  **Cleanup (Status Update):** Once the entry function returns, the stub intercepts the exit. It marks the thread's status as `DEAD` so the scheduler will no longer place it in the run queue.
3.  **Teardown:** It attempts to clean up the thread object and free the allocated stack memory.

> **⚠️ Known Architecture Bug (V1 Limitations):** 
> In this implementation, the stub attempts to call `kfree` on its own stack right before calling `yield()`. Freeing the memory space that the current `ESP` relies on is highly unsafe and creates a race condition. This bug is patched in the v2 scheduler via the introduction of the deferred **Zombie Reaper Thread**.

## 4. Scheduler Responsibilities

The scheduler coordinates these mechanisms via two core state structures:
*   **Run Queue:** A linked list (or queue) of all currently runnable threads.
*   **Current Thread:** A global pointer tracking the actively executing thread.

When a hardware interrupt fires (via the PIT timer) or a thread explicitly calls `yield()`, the scheduler takes control over the execution flow:

1.  **Re-queue:** It evaluates the `current_thread`. If the thread is still marked as runnable (i.e., it hasn't exited or gone to sleep), it is placed at the back of the run queue.
2.  **Select:** It pops the next available thread off the front of the run queue.
3.  **Update State:** It updates the global `current_thread` pointer to track the incoming thread.
4.  **Execute Switch:** It invokes the assembly stub, passing the address of the outgoing thread's stack pointer and the value of the incoming thread's stack pointer: `context_switch(&old_thread->esp, new_thread->esp)`.

## 5. Misc: The Sleep Queue

To allow threads to yield execution for a specific duration without busy-waiting, the scheduler implements a basic sleep queue. 

*   **Structure:** A linked list of sleeping threads, sorted strictly by their `wake_at` target tick.
*   **Timestamp:** The `wake_at` value is a 64-bit unsigned integer (`uint64_t`). This guarantees the system can handle massively long sleep durations (years of kernel uptime) without the tick counter overflowing and waking threads prematurely.
*   **Execution:** Because the queue is sorted chronologically, the PIT `IRQ0` handler only ever needs to check the **Head Node**. If the system's current uptime tick matches or exceeds the head's `wake_at` value, that thread is popped off the sleep queue, marked as runnable, and pushed into the main run queue. 

This ensures the hardware timer interrupt remains as fast as possible ($O(1)$) because it never iterates the list during a tick.
