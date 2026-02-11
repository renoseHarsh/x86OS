# Physical Memory Manager

The physical memory manager is responsible for managing the physical memory of the system. It keeps track of which pages of memory are free and which are allocated, and it provides functions for allocating and freeing memory.

I choose Buddy Allocator as the physical memory manager. The buddy allocator is a simple and efficient memory allocator that works by dividing memory into blocks of size 2^k, where k is a non-negative integer. When a block of memory is allocated, it is split into two smaller blocks until the desired size is reached. When a block of memory is freed, it is merged with its buddy if the buddy is also free.

## Ways to implement the buddy allocator include
1. Bitmap & Freelist: This method uses the Freelist to keep track of free blocks of memory we use an array of linked lists, where each linked list corresponds to a specific block size (2^k). Each node in the linked list represents a free block of memory of that size. When a block is allocated, it is removed from the corresponding linked list. When a block is freed, it is added back to the linked list. The bitmap is used to keep track of which blocks are allocated and which are free, this usesful for quickly checking the status of a block and for mergiing blocks when they are freed. The time complexity of for allocating a block of memory is O(log n) where n is the total number of blocks, and the time complexity for freeing a block of memory is also O(log n) due to the merging process, but because orders of blocks are limited, it can be considered O(1) in practice.

2. Array of 4kib Blocks & Freelist: We can create a struct called `Page` that represents a block of memory, which contains a pointer to the next and previous block of memories,  flag to to track the status of the block (allocated or free) and order which specifies the order of the block. We can then create an array of these blocks, where each block represents a 4KiB page of memory. We can then insert the free blocks into an array of linked lists, where each linked list corresponds to a specific block size (2^k). When a block of memory is allocated, it is removed from the corresponding linked list. When a block of memory is freed, it is added back to the linked list. We can use the array of 4kib blocks to quickly check the status of a neighboring block to merge it.

Why I picked the second method:
1. Cache: In the first method the next and prev pointer are stored in the free blocks, which means when traversing over the freelist we are accesing memory huge blocks away, cause cache miss, also this loads that physciall memory into the cache, which is not needed for the buddy allocator, and can cause more cache misses when accessing the actual data stored in the allocated blocks. In the second method, the next and prev pointers are stored in a separate array of blocks, which allows for better cache locality and reduces cache misses when traversing the freelist.
2. Tracking: In the first method, during freeing we are not aware as to what order of block is being free, but we can check our struct in the array to know the order of the block, this is useful for merging blocks when they are freed, this makes it so we don't have to pass the order during freeing.


## Pre-processing
We need a memory map, which will contain Page structs that would represent the entire physical memory, we can read a memory map from the bootloader, which will give us the starting address and size of each memory region, we can then use this information to initialize our array of Page structs, marking the blocks that are available for allocation as free and the blocks that are reserved or in use as allocated. We can also use the memory map to determine the total amount of physical memory available and to set up our freelist accordingly.

## Flags
1. **Allocated**: The block is currently free
2. **Free**: The block is currently allocated
3. **Kernel**: The block is reserved for the kernel
4. **Head**: The block is the head of a larger block of memory, which means that for a larger order block let's say 2nd order which means a 8kib block this flag will be set for the first 4kib block of the 8kib block.
5. **Tail**: The block is the tail of a larger block of memory, which means that for a larger order block let's say 2nd order which means a 8kib block this flag will be set for the second 4kib block of the 8kib block.
