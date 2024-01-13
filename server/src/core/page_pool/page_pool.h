#pragma once

#include "core/io/linked_pages.h"
#include <stdint.h>

#define sizeof_Page_Header (sizeof(int64_t) * 7)

typedef struct chblix{
    int64_t block_idx;
    int64_t chunk_idx;
} chblix_t;

typedef struct chunk {
    linked_page_t lp_header;
    int64_t page_index;
    int64_t capacity;
    int64_t num_of_free_blocks;
    int64_t num_of_used_blocks;
    int64_t next;
    int64_t prev_page;
    int64_t next_page;
} chunk_t;

typedef struct page_pool {
    linked_page_t lp_header;
    int64_t current_idx;
    int64_t head;
    int64_t tail;
    int64_t block_size;
    int64_t wait; // parray index
} page_pool_t;

typedef enum {PPL_SUCCESS = 0, PPL_FAIL = -1, PPL_EMPTY = 1} page_pool_status_t;

#define CHBLIX_FAIL (chblix_t){.block_idx = -1, .chunk_idx = -1}
chblix_t chblix_fail(void);
int chblix_cmp(const chblix_t* chblix1, const chblix_t* chblix2);

#define page_pool_index(ppl) (ppl->lp_header.page_index)

int64_t ppl_chunk_init(page_pool_t* ppl);
chunk_t* ppl_create_page(page_pool_t* ppl);
chunk_t* ppl_load_chunk(int64_t chunk_index);
int ppl_delete_chunk(chunk_t* chunk);
int ppl_write_block_nova(page_pool_t* ppl, const chblix_t* chblix, void* src, int64_t size, int64_t src_offset);
int ppl_write_block(int64_t ppidx, const chblix_t* chblix, void* src, int64_t size, int64_t src_offset);
int ppl_read_block(int64_t ppidx, const chblix_t* chblix, void* dest,  int64_t size, int64_t src_offset);
int ppl_read_block_nova(page_pool_t* ppl, linked_page_t* lp, const chblix_t* chblix, void* dest,  int64_t size, int64_t src_offset);
int ppl_pool_expand(page_pool_t* ppl);
chblix_t ppl_alloc_nova(page_pool_t* ppl);
chblix_t ppl_alloc(int64_t ppidx);
int ppl_pool_reduce(page_pool_t* ppl, chunk_t* page);
int ppl_dealloc_nova(page_pool_t* ppl, chblix_t* chblix);
int ppl_dealloc(int64_t ppidx, chblix_t* chblix);
int64_t ppl_init(int64_t block_size);
page_pool_t* ppl_load(int64_t start_page_index);
int ppl_destroy(int64_t pplidx);
