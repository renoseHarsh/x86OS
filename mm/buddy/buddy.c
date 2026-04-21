#include "buddy/buddy.h"
#include "buddy/test_buddy.h"
#include "kprintf.h"
#include "layout.h"
#include "panic.h"
#include "pmm.h"
#include "utils.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define real_page_index(addr) ((addr) >> BASE_SHIFT)
#define page_map_index(ptr) ((ptr) - page_map)
#define buddy_index(idx) ((idx) ^ (1 << page_map[(idx)].order))
#define page_map_real(top) (0x1000 * page_map_index(top))

extern memory_map_t memory_map[];

Page *free_area[MAX_BUDDY_ORDER + 1] = { NULL };
Page *page_map;
size_t page_n;
size_t free_pages_count, used_pages_count;

static void make_pages()
{
    uint32_t total_size = (uint32_t)memory_map[3].addr + memory_map[3].size;
    page_n = (total_size + (0x1000 - 1)) >> 12;
    uint32_t map_size = page_n * sizeof(Page);
    uint32_t pages_needed = (map_size + (0x1000 - 1)) >> 12;
    page_map = (Page *)P2V(pmm_alloc_page());
    pages_needed--;
    while (pages_needed--)
        pmm_alloc_page();

    for (size_t i = 0; i < page_n; i++) {
        page_map[i].flags = P_KERNEL;
        page_map[i].order = 0;
        page_map[i].node.next = NULL;
        page_map[i].node.prev = NULL;
    }
}

static void add_pages()
{
    uint32_t start = get_placement_addr();
    uint32_t end = memory_map[3].addr + memory_map[3].size;

    while (start < end) {
        bool found = false;
        for (int i = MAX_BUDDY_ORDER; i >= 0; i--) {
            uint32_t size = BASE_SIZE << i;
            if (IS_ALIGNED(start, size) && start + size <= end) {
                int idx = real_page_index(start);
                free_pages_count += (1 << i);
                page_map[idx].order = i;
                page_map[idx].flags = P_FREE;
                int tail_pages = 1 << i;
                for (int tail = 1; tail < tail_pages; tail++) {
                    page_map[idx + tail].flags = P_TAIL;
                }
                push_node((Node **)&free_area[i], &page_map[idx].node);
                start += (BASE_SIZE << i);
                found = true;
                break;
            }
        }
        if (!found) {
            kprintf("0x%x, Last block wasn't aligned\n", start);
            break;
        }
    }
}

size_t get_order_addr(uintptr_t addr)
{
    size_t idx = real_page_index(addr);
    return page_map[idx].order;
}

void *alloc_pages(const int order)
{
    int cur_order = order;
    for (; !free_area[cur_order] && cur_order <= MAX_BUDDY_ORDER;
         cur_order++) {
    }
    if (cur_order > MAX_BUDDY_ORDER) {
        kprintf("Out of memory");
        kernel_panic();
    }

    Page *top = (Page *)pop_node((Node **)&free_area[cur_order]);
    while (cur_order > order) {
        cur_order--;
        top->order = cur_order;
        top->flags = P_USED;
        int index = page_map_index(top);
        Page *sibling = &page_map[buddy_index(index)];
        sibling->order = cur_order;
        sibling->flags = P_FREE;
        push_node((Node **)&free_area[cur_order], &sibling->node);
    }
    top->order = cur_order;
    top->flags = P_USED;
    used_pages_count += (1 << order);
    free_pages_count -= (1 << order);
    return (void *)page_map_real(top);
}

void free_pages(void *addr)
{
    uintptr_t real_addr = (uintptr_t)addr;
    int index = real_page_index(real_addr);
    Page *top = &page_map[index];
    int cur_order = top->order;
    used_pages_count -= (1 << cur_order);
    free_pages_count += (1 << cur_order);
    while (cur_order < MAX_BUDDY_ORDER) {
        int b_index = buddy_index(index);
        Page *buddy = &page_map[b_index];
        if (buddy->flags == P_FREE && buddy->order == cur_order) {
            remove_node((Node **)&free_area[cur_order], &buddy->node);
            if (b_index < index) {
                Page *temp = buddy;
                buddy = top;
                top = temp;
                index = b_index;
            }
            top->order = cur_order + 1;
            top->flags = P_FREE;
            buddy->flags = P_TAIL;
            cur_order++;
        } else
            break;
    }
    top->order = cur_order;
    top->flags = P_FREE;
    push_node((Node **)&free_area[cur_order], &top->node);
}

void init_buddy()
{
    free_pages_count = used_pages_count = 0;
    make_pages();
    add_pages();
    // test_buddy();
}
