# Kernel Heap Allocator

The kernel heap provides dynamic memory allocation for kernel structures and operations. It sits as an abstraction layer above the Virtual Memory Manager (VMM) and Physical Memory Manager (PMM).

## Allocation Strategy: Segregated Free Lists

When designing the heap, two primary models were considered:
*   **Slab Allocator:** Pre-allocates fixed-size caches for specific object types. It is incredibly fast and eliminates external fragmentation, but requires knowing object sizes ahead of time. It can also suffer from internal fragmentation if objects are rounded up to the slab size.
*   **Dynamic Freelist:** Maintains lists of free memory chunks. It dynamically splits and merges blocks based on requested sizes. This suffers from external fragmentation (due to metadata headers) but offers maximum flexibility.

**Decision:** The heap is implemented using a **Segregated Freelist** approach. Because the exact memory usage patterns of the kernel are not yet strictly defined, the dynamic flexibility of a freelist is required. If specific kernel structures (like Thread Control Blocks) become high-traffic, dedicated Slab allocators may be introduced later for those specific objects to optimize speed and reduce fragmentation.

---

## Core Structures & Requirements

### 1. The Chunk Header
Every block of memory (both free and allocated) is tracked by a `Chunk` header. To minimize metadata overhead, the struct utilizes a C `union`. 
*   When the chunk is allocated, the memory immediately following the header is treated as raw `data`.
*   When the chunk is free, that exact same memory space is used to store a `Node` pointer to track the next free chunk in the list.
```c
typedef struct {
    Node all;       // Tracks contiguous chunks in virtual memory (for coalescing)
    bool used;      // Allocation status flag
    union {
        char data[0]; // Flexible array member for allocated memory
        Node free;    // Pointer to the next free chunk in the list
    };
} Chunk;
```

### 2. Segregated List Array
Instead of a single freelist, the heap maintains an array of freelists categorized by size (powers of 2).
* **MinimumAllocation:** Order 3 (8 bytes).
* **Maximum Heap Allocation:** Order 11 (2048 bytes).
* The array indices are shifted by the smallest allocatable chunk. Requests for memory larger than 2048 bytes bypass the heap entirely and are redirected directly to the Page Allocator (PMM).

---

### Operations
**`kmalloc` (Allocation)**

`kmalloc` serves as the entry point for dynamic memory requests.
1. **Size Check:** If the requested size exceeds 2048 bytes, the request is delegated to the VMM/Page Allocator.
2. **List Indexing:** The requested size is rounded up to the nearest power of 2 to determine the correct freelist index.
3. **Chunk Retrieval & Splitting:** The first available chunk is popped from the list. If the chunk is significantly larger than requested, it is split. The requested portion is marked as used, and the remainder is formatted as a new free chunk and pushed into the appropriate lower-order freelist.
4. **Heap Expansion:** If no suitable chunks exist in the lists, the heap requests additional pages from the VMM. The VMM maps new virtual pages, backed by physical frames from the Buddy Allocator.

**Time Complexity:** Generally O(1) for standard allocations due to the segregated array providing direct access to best-fit blocks. Splitting and pushing are also constant-time operations. Heap expansion incurs an O(logN) penalty as the PMM's Buddy Allocator must split high-order memory blocks.

### kfree (Reclamation)
`kfree` returns memory to the system.
1. **Origin Check:** It first verifies if the memory was allocated via the heap or directly from the Page Allocator (sizes > 2048 bytes). If the latter, it is passed to the VMM for page-level teardown.
2. **Virtual Coalescing:** The allocator checks the adjacent chunks in virtual memory using the all Node tracker. If the neighboring virtual chunks are also free, they are merged into a single, larger free chunk to combat external fragmentation.
3. **List Insertion:** The newly freed (and potentially coalesced) chunk is pushed into the appropriate segregated freelist based on its final size.
4. **Page Reclamation:** If a coalesced block results in entirely unused virtual pages, the heap can optionally return those frames to the system.

**Time Complexity:** Generally O(1). Determining the correct freelist is a simple logarithmic bit-shift of the size, and coalescing requires basic pointer reassignment.

### References & Acknowledgments
* **Implementation Base:** The core architecture and logic for this segregated freelist are heavily inspired by and adapted from [Mrvn's LinkedList Bucket Heap Implementation](https://wiki.osdev.org/User:Mrvn/LinkedListBucketHeapImplementation) on the OSDev Wiki.
