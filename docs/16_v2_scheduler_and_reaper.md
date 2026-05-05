# Scheduler V2 & The Zombie Reaper

With the introduction of Ring 3 user-space programs, the V1 kernel thread scheduler required a complete overhaul. It needed to support hardware-isolated address spaces, protect its internal queues from race conditions, and solve a critical memory lifecycle bug.

## 1. Process Memory Isolation (CR3 Swapping)

In V1, all threads were kernel threads sharing a single Page Directory. In V2, user-space processes require entirely isolated virtual memory spaces. 

To enable this, a Page Directory pointer was added to the Thread Control Block (TCB). When the scheduler decides to switch from `Thread A` to `Thread B`, it performs the following checks:

1.  **Directory Comparison:** It checks if `next_thread->page_dir == current_thread->page_dir`.
2.  **Hardware Context Switch:** If the directories are different, the scheduler extracts the physical address of the new Page Directory and writes it directly to the CPU's `CR3` control register.
3.  **TLB Flush:** Writing to `CR3` automatically flushes the CPU's Translation Lookaside Buffer (TLB), ensuring the new thread cannot accidentally access cached memory translations from the old thread.

*(Note: To make this possible, the Higher-Half Kernel space must be perfectly mapped into every single user process Page Directory. This ensures that when `CR3` is swapped, the scheduler's own code and Ring 0 stacks do not suddenly unmap, which would instantly crash the CPU).*

## 2. Atomicity and Queue Protection

The scheduler relies on shared data structures, primarily the Main Run Queue and the Sleep Queue. Because context switches are driven asynchronously by the PIT hardware timer (`IRQ0`), a critical race condition existed.

If a thread was in the middle of modifying the Run Queue (e.g., yielding and moving its TCB to the back of the linked list) and a timer interrupt fired, the interrupt handler would attempt to run the scheduler and modify the exact same queue simultaneously. This would mutilate the queue's state, resulting in lost threads or memory corruption.

**The Solution:** All queue manipulations and scheduler operations are now strictly wrapped in atomic blocks (`atomic_on` / `atomic_off`). By clearing the Interrupt Enable flag (`cli`) before modifying the queues and setting it afterward (`sti`), the scheduler guarantees it can perform its operations in one cohesive pass without being preempted.

## 3. Fixing the Teardown Bug: The Zombie Reaper

A fundamental rule of kernel development is that **a thread cannot free its own stack**. 

In the V1 implementation, a dying thread would call `kfree()` on its own Thread Control Block and stack, and then call `yield()`. Even with this bug, the program often successfully context-switched because the memory hadn't been overwritten yet. However, it was a ticking time bomb: if a hardware interrupt fired *after* `kfree()` was called but *before* the context switch finished, the CPU would push the interrupt frame onto memory that was technically unallocated, risking catastrophic kernel panics or data corruption.

To solve this, the V2 scheduler implements a deferred cleanup model:

1.  **State Change:** When a thread exits, it does *not* free its own memory. It simply changes its status to `ZOMBIE` and yields the CPU.
2.  **The Reaper Thread:** A dedicated Ring 0 background thread—the Zombie Reaper—is spawned during boot.
3.  **Garbage Collection:** The Reaper wakes up periodically, iterates through the global list of threads, and looks for `ZOMBIE` statuses. Because a Zombie thread is definitively no longer running on the CPU, the Reaper can safely call `kfree()` on the dead thread's Ring 0 stack, Ring 3 stack, and TCB without any risk of memory corruption.
