#include "buddy/buddy.h"
#include "heap.h"
#include "heap_test.h"
#include "kprintf.h"
#include "layout.h"
#include "node.h"
#include "paging.h"
#include "string.h"
#include "utils.h"
#include <stdbool.h>

typedef struct {
    Node all;
    bool used;
    union {
        char data[0];
        Node free;
    };
} Chunk;

enum {
    HEADER_SIZE = __builtin_offsetof(Chunk, data),
    MIN_SIZE = sizeof(Node),
    NUM_SIZES = 9,
    ORDER_OFFSET = 3,
    ALIGN = 4
};

Node *free_bucket[NUM_SIZES] = { NULL };
Chunk *first, *last;

size_t mem_free = 0, mem_used = 0, mem_meta = 0;
extern size_t used_pages_count;

int log_two(size_t size)
{
    int order = -1;
    while (size) {
        size >>= 1;
        order++;
    }
    if (order == -1)
        return 0;
    return order;
}

size_t get_total_size(Chunk *chunk)
{
    return (uintptr_t)chunk->all.next - (uintptr_t)chunk;
}

size_t get_chunk_size(Chunk *chunk)
{
    return get_total_size(chunk) - HEADER_SIZE;
}

int get_bucket_index(Chunk *chunk)
{
    size_t size = get_chunk_size(chunk);
    int order = log_two(size);
    return order - ORDER_OFFSET;
}

int get_alloc_order(size_t size)
{
    int order = log_two(size);
    if (IS_ALIGNED(size, size))
        return order - ORDER_OFFSET;
    return order + 1 - ORDER_OFFSET;
}

Chunk *create_chunk()
{
    Chunk *f_block = (Chunk *)P2V(alloc_pages(0));
    kmemset(f_block, 0, PAGE_SIZE);
    Chunk *m_block = f_block + 1;
    Chunk *l_block = (Chunk *)((uintptr_t)f_block + PAGE_SIZE) - 1;

    f_block->used = true;
    m_block->used = false;
    l_block->used = true;

    insert_node(&f_block->all, &m_block->all);
    insert_node(&m_block->all, &l_block->all);

    mem_meta += get_total_size(f_block); // for f_block
    mem_meta += get_total_size(f_block); // for l_block
    mem_meta += HEADER_SIZE;             // for m_block
    mem_free += get_chunk_size(m_block);

    int order = get_bucket_index(m_block);

    push_node(&free_bucket[order], &m_block->free);

    if (!first) {
        first = f_block;
        last = l_block;
    } else {
        insert_node(&last->all, &f_block->all);
        last = l_block;
    }

    return f_block;
}

Chunk *pop_free_chunk(int order)
{
    if (order < 0 || order >= NUM_SIZES) {
        return NULL;
    }
    Node *node = pop_node(&free_bucket[order]);
    return (Chunk *)((uintptr_t)node - HEADER_SIZE);
}

void *kmalloc(size_t size)
{
    if (size <= 0)
        return NULL;

    if (size < MIN_SIZE)
        size = MIN_SIZE;

    size_t alloc_size = ALIGN_UP(size, ALIGN);
    int order = get_alloc_order(alloc_size);
    size_t total_size = alloc_size + HEADER_SIZE;

    Chunk *chunk = NULL;

    for (int i = order; i < NUM_SIZES; i++) {
        if (free_bucket[i]) {
            chunk = pop_free_chunk(i);
            break;
        }
    }

    if (!chunk)
        return NULL;

    chunk->used = true;

    size_t chunk_size = get_chunk_size(chunk);
    mem_free -= chunk_size;

    if (chunk_size - alloc_size >= MIN_SIZE + HEADER_SIZE) {
        Chunk *nChunk = (Chunk *)((uintptr_t)chunk + total_size);
        nChunk->used = false;
        insert_node(&chunk->all, &nChunk->all);
        mem_meta += HEADER_SIZE;
        int nOrder = get_bucket_index(nChunk);
        push_node(&free_bucket[nOrder], &nChunk->free);
        mem_free += get_chunk_size(nChunk);
    }

    mem_used += get_chunk_size(chunk);

    return chunk->data;
}

Chunk *get_chunk_offset(void *ptr)
{
    return (Chunk *)((uintptr_t)ptr - HEADER_SIZE);
}

void kfree(void *ptr)
{
    Chunk *chunk = get_chunk_offset(ptr);
    Chunk *next = (Chunk *)chunk->all.next;
    Chunk *prev = (Chunk *)chunk->all.prev;

    mem_used -= get_chunk_size(chunk);

    if (!next->used) {
        remove_node(&free_bucket[get_bucket_index(next)], &next->free);
        mem_free -= get_chunk_size(next);
        remove_node(&(first->all.next), &next->all);
        mem_meta -= HEADER_SIZE;
    }
    if (!prev->used) {
        remove_node(&free_bucket[get_bucket_index(prev)], &prev->free);
        mem_free -= get_chunk_size(prev);
        remove_node(&(first->all.next), &chunk->all);
        mem_meta -= HEADER_SIZE;
        chunk = prev;
    }

    chunk->used = false;

    push_node(&free_bucket[get_bucket_index(chunk)], &chunk->free);
    mem_free += get_chunk_size(chunk);
}

void init_kmalloc()
{
    first = create_chunk();
    last = (Chunk *)first->all.next->next;
    // test_heap();
}
