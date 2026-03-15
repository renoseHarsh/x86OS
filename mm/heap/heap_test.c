#include "heap/heap.h"
#include "heap/heap_test.h"
#include "kprintf.h"
#include "random.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

extern size_t mem_free, mem_used, mem_meta, used_pages_count;
extern void print_stats();

#define SEED 2324
#define MAX_MEMORY 500
#define MAX_BLOCKS 500
#define MAX_ITERATIONS 1000
size_t tracking = 0;
size_t ini_mem_used, ini_mem_free, ini_mem_meta;

typedef struct {
    void *ptr;
    size_t size;
    uint8_t pattern;
} Block;

static Block blocks[MAX_BLOCKS];

static void fill_pattern(void *ptr, size_t size, uint8_t pattern)
{
    uint8_t *p = (uint8_t *)ptr;
    for (size_t i = 0; i < size; i++) {
        p[i] = pattern;
    }
}

static bool check_pattern(void *ptr, size_t size, uint8_t pattern)
{
    uint8_t *p = (uint8_t *)ptr;
    for (size_t i = 0; i < size; i++) {
        if (p[i] != pattern) {
            return false;
        }
    }
    return true;
}

void allocate()
{
    size_t size = (krand() % MAX_MEMORY) + 1;

    size_t idx = krand() % MAX_BLOCKS;

    while (blocks[idx].ptr != NULL) {
        idx = krand() % MAX_BLOCKS;
    }

    Block *block = &blocks[idx];
    block->ptr = kmalloc(size);
    block->size = size;
    block->pattern = (uint8_t)(krand() & 0xFF);

    if (block->ptr) {
        fill_pattern(block->ptr, block->size, block->pattern);
        tracking++;
    }
}

void deallocate()
{
    size_t idx = krand() % MAX_BLOCKS;

    while (blocks[idx].ptr == NULL) {
        idx = krand() % MAX_BLOCKS;
    }

    Block *block = &blocks[idx];

    if (!check_pattern(block->ptr, block->size, block->pattern)) {
        kprintf("Memory corruption detected in block %d!\n", idx);
    }

    kfree(block->ptr);
    block->ptr = NULL;
    block->size = 0;
    block->pattern = 0;
    tracking--;
}

static bool check_state()
{

    if (mem_used + mem_meta + mem_free != (used_pages_count * 0x1000)) {
        kprintf(
            "Memory accounting error: used=%d, free=%d, meta=%d\n", mem_used,
            mem_free, mem_meta
        );
        return false;
    }
    return true;
}

static bool check_ini()
{
    if (mem_used != ini_mem_used) {
        kprintf(
            "Memory used mismatch: initial=%d, final=%d\n", ini_mem_used,
            mem_used
        );
        return false;
    }
    if (mem_free != ini_mem_free) {
        kprintf(
            "Memory free mismatch: initial=%d, final=%d\n", ini_mem_free,
            mem_free
        );
        return false;
    }
    if (mem_meta != ini_mem_meta) {
        kprintf(
            "Memory meta mismatch: initial=%d, final=%d\n", ini_mem_meta,
            mem_meta
        );
        return false;
    }
    return true;
}

void test_heap()
{

    kprintf("Starting heap test...\n");
    ksrand(SEED);

    ini_mem_used = mem_used;
    ini_mem_free = mem_free;
    ini_mem_meta = mem_meta;

    for (int i = 0; i < MAX_BLOCKS; i++) {
        blocks[i].ptr = NULL;
        blocks[i].size = 0;
        blocks[i].pattern = 0;
    }

    for (int i = 0; i < MAX_ITERATIONS; i++) {
        int action = krand() & 1; // 0 = alloc, 1 = free

        if (tracking == 0)
            action = 0;
        if (tracking >= MAX_BLOCKS)
            action = 1;

        if (action == 0) {
            allocate();
        } else {
            deallocate();
        }

        if (!check_state()) {
            return;
        }
    }

    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (blocks[i].ptr != NULL) {
            kfree(blocks[i].ptr);
            blocks[i].ptr = NULL;
            blocks[i].size = 0;
            blocks[i].pattern = 0;
            tracking--;
        }
    }

    if (check_ini() && tracking == 0) {
        kprintf("Heap test passed!\n");
    }
}
