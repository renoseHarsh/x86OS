#include "buddy/buddy.h"
#include "buddy/freelist.h"
#include "buddy/test_buddy.h"
#include "kprintf.h"
#include "layout.h"
#include "random.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define SEED 7465
#define MAX_BLOCKS 500
#define ITERATIONS 1000

#define real_page_index(addr) ((addr) >> BASE_SHIFT)
#define page_map_index(ptr) ((ptr) - page_map)
#define buddy_index(idx) ((idx) ^ (1 << page_map[(idx)].order))

typedef struct {
    void *ptr;
    size_t order;
    uint8_t pattern;
} Block;

extern Page *free_area[];
extern Page *page_map;
extern size_t page_n, free_pages_count, used_pages_count;

static Block blocks[MAX_BLOCKS];
int init_state[MAX_BUDDY_ORDER + 1];
size_t total_allocations = 0;

static void fill_pattern(void *ptr, size_t order, uint8_t pattern)
{
    size_t pages = 1 << order;
    for (size_t i = 0; i < pages; i++) {
        uint8_t *p = (uint8_t *)((uintptr_t)ptr + i * 0x1000);
        for (size_t j = 0; j < 0x1000; j++) {
            p[j] = pattern;
        }
    }
}

static bool check_pattern(void *ptr, size_t order, uint8_t pattern)
{
    size_t pages = 1 << order;
    for (size_t i = 0; i < pages; i++) {
        uint8_t *p = (uint8_t *)((uintptr_t)ptr + i * 0x1000);
        for (size_t j = 0; j < 0x1000; j++) {
            if (p[j] != pattern) {
                return false;
            }
        }
    }
    return true;
}

bool check_state(Page *page, int order, int flag)
{
    int pages = 1 << page->order;

    if (page->flags != flag || page->order != order)
        return false;

    for (int i = 1; i < pages; i++) {
        if (page[i].flags != P_TAIL) {
            return false;
        }
    }

    return true;
}

bool check_map(size_t *free_count, size_t *used_count)
{
    for (size_t i = 0; i < page_n; i++) {
        if (page_map[i].flags == P_KERNEL)
            continue;

        if (page_map[i].flags == P_FREE) {
            size_t pages = 1 << page_map[i].order;
            *free_count += pages;
            for (size_t j = 1; j < pages; j++) {
                if (page_map[i + j].flags != P_TAIL) {
                    kprintf("Page %u: Tail check failed\n", i + j);
                    return false;
                }
            }
            i += pages - 1;
        } else if (page_map[i].flags == P_USED) {
            size_t pages = 1 << page_map[i].order;
            *used_count += pages;
            for (size_t j = 1; j < pages; j++) {
                if (page_map[i + j].flags != P_TAIL) {
                    kprintf("Page %u: Tail check failed\n", i + j);
                    return false;
                }
            }
            i += pages - 1;
        }
    }
    return true;
}

void debug_check(Page *page, size_t order, int flag)
{
    int pages = 1 << page->order;
    if (page->flags != flag || page->order != order) {
        kprintf(
            "Failed cause flag or order mismatch, flags %d == %d, order %d == "
            "%d\n",
            page->flags, flag, page->order, order
        );
        return;
    }

    for (int i = 1; i < pages; i++) {
        if (page[i].flags != P_TAIL) {
            kprintf(
                "Failed cause tail check failed at page %u\n",
                page_map_index(page) + i
            );
            return;
        }
    }
}

bool check_free_areas(size_t *free_area_count)
{
    for (int i = 0; i <= MAX_BUDDY_ORDER; i++) {
        Page *cur = free_area[i];
        while (cur) {
            if (!check_state(cur, i, P_FREE)) {
                kprintf("Free area order %u: State check failed\n", i);
                return false;
            }

            if (i < MAX_BUDDY_ORDER) {
                size_t idx = page_map_index(cur);
                size_t buddy_idx = buddy_index(idx);

                if (buddy_idx < page_n) {
                    Page *buddy = &page_map[buddy_idx];
                    if (buddy->flags == P_FREE && buddy->order == i) {
                        kprintf(
                            "Free area order %u: Buddy check failed for page "
                            "%u and buddy %u\n",
                            i, idx, buddy_idx
                        );
                        return false;
                    }
                }
            }

            *free_area_count += (1 << i);
            cur = cur->next;
        }
    }

    return true;
}

bool check_allocations(size_t *expected_used_count)
{
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (blocks[i].ptr) {
            size_t idx = real_page_index(V2P(blocks[i].ptr));
            if (!check_state(&page_map[idx], blocks[i].order, P_USED)) {
                kprintf("Block %d: State check failed\n", i);
                return false;
            }
            *expected_used_count += (1 << blocks[i].order);
        }
    }

    return true;
}

bool check()
{
    size_t free_count = 0, used_count = 0;
    if (!check_map(&free_count, &used_count)) {
        kprintf("Map check failed\n");
        return false;
    }

    size_t free_area_count = 0;
    if (!check_free_areas(&free_area_count)) {
        kprintf("Free area check failed\n");
        return false;
    }
    size_t expected_used_count = 0;
    if (!check_allocations(&expected_used_count)) {
        kprintf("Allocation check failed\n");
        return false;
    }

    // if (free_count != free_area_count) {
    //     kprintf(
    //         "Free count mismatch: map %u, free areas %u\n", free_count,
    //         free_area_count
    //     );
    //     return false;
    // }

    if (used_count != expected_used_count) {
        kprintf(
            "Used count mismatch: map %u, allocations %u\n", used_count,
            expected_used_count
        );
        return false;
    }

    if (free_count != free_pages_count) {
        kprintf(
            "Free count mismatch: map %u, global count %u\n", free_count,
            free_pages_count
        );
        return false;
    }

    if (used_count != used_pages_count) {
        kprintf(
            "Used count mismatch: map %u, global count %u\n", used_count,
            used_pages_count
        );
        return false;
    }

    if (free_area_count != free_pages_count) {
        kprintf(
            "Free area count mismatch: free areas %u, global count %u\n",
            free_area_count, free_pages_count
        );
        return false;
    }

    if (expected_used_count != used_pages_count) {
        kprintf(
            "Used count mismatch: allocations %u, global count %u\n",
            expected_used_count, used_pages_count
        );
        return false;
    }

    return true;
}

void print_all_len()
{
    for (int i = 0; i <= MAX_BUDDY_ORDER; i++) {
        int count = 0;
        Page *cur = free_area[i];
        while (cur) {
            count++;
            cur = cur->next;
        }
        kprintf("Free area order %d: %d blocks\n", i, count);
    }
}

void print_status()
{
    kprintf(
        "Free pages: %u, Used pages: %u\n", free_pages_count, used_pages_count
    );
}

bool verify_final_state()
{
    for (int i = 0; i <= MAX_BUDDY_ORDER; i++) {
        int len = 0;
        for (Page *cur = free_area[i]; cur; cur = cur->next)
            len++;
        if (init_state[i] != len) {
            kprintf("State Not Matching\n");
            return false;
        }
    }
    return true;
}

void do_alloc()
{
    size_t order = krand() % (MAX_BUDDY_ORDER + 1);
    char pattern = krand() % 256;
    char *ptr = (char *)P2V(alloc_pages(order));
    if (!ptr) {
        kprintf("failed\n");
        return;
    }

    fill_pattern(ptr, order, pattern);

    size_t idx = krand() % MAX_BLOCKS;
    while (blocks[idx].ptr)
        idx = krand() % MAX_BLOCKS;

    blocks[idx].ptr = ptr;
    blocks[idx].order = order;
    blocks[idx].pattern = pattern;
}

bool do_free()
{
    size_t idx = krand() % MAX_BLOCKS;

    while (!blocks[idx].ptr)
        idx = krand() % MAX_BLOCKS;

    Block block = blocks[idx];

    if (!check_pattern(block.ptr, block.order, block.pattern)) {
        kprintf("Memory corruption\n");
        return false;
    }
    free_pages((void *)V2P(blocks[idx].ptr));
    blocks[idx].ptr = 0x0;
    return true;
}

void do_random_op()
{
    bool alloc = krand() % 2;

    if (total_allocations >= MAX_BLOCKS)
        alloc = false;
    if (total_allocations == 0)
        alloc = true;
    if (free_pages_count == 0)
        alloc = false;

    if (alloc) {
        do_alloc();
        total_allocations++;
    } else {
        if (!do_free()) {
            kprintf("Free operation failed\n");
        }
        total_allocations--;
    }
}

void do_path_op(int i, int op, int idx, int order)
{
    if (op == 1) {
        char pattern = krand() % 256;
        char *ptr = (char *)P2V(alloc_pages(order));
        if (!ptr) {
            kprintf("failed\n");
            return;
        }

        fill_pattern(ptr, order, pattern);

        blocks[idx].ptr = ptr;
        blocks[idx].order = order;
        blocks[idx].pattern = pattern;
    } else {
        Block block = blocks[idx];

        if (!check_pattern(block.ptr, block.order, block.pattern)) {
            kprintf("Memory corruption\n");
            return;
        }
        free_pages((void *)V2P(blocks[idx].ptr));
        blocks[idx].ptr = 0x0;
    }
    if (!check()) {
        kprintf(
            "%d : State check failed after operation %d on block %d\n", i, op,
            idx
        );
    }
}

void free_all_allocation()
{
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (blocks[i].ptr) {
            if (!check_pattern(
                    blocks[i].ptr, blocks[i].order, blocks[i].pattern
                )) {
                kprintf("Memory corruption in block %d\n", i);
                continue;
            }
            free_pages((void *)V2P(blocks[i].ptr));
            blocks[i].ptr = 0x0;
        }
    }
}

void test_buddy()
{
    kprintf("Starting buddy test...\n");
    ksrand(SEED);

    for (int i = 0; i < MAX_BLOCKS; i++) {
        blocks[i].ptr = 0x0;
    }

    for (int i = 0; i <= MAX_BUDDY_ORDER; i++) {
        int len = 0;
        for (Page *cur = free_area[i]; cur; cur = cur->next)
            len++;
        init_state[i] = len;
    }

    for (int i = 0; i < ITERATIONS; i++) {
        do_random_op();

        if (total_allocations == 0) {
            if (!verify_final_state()) {
                kprintf(
                    "Final state verification failed at iteration %d\n", i
                );
                return;
            }
        }
        if (!check()) {
            kprintf("State check failed at iteration %d\n", i);
            return;
        }
    }

    free_all_allocation();

    if (verify_final_state()) {
        kprintf("Buddy test passed successfully!\n");
    }
}
