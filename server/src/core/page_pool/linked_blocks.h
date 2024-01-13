#pragma once
#include "page_pool.h"
#include <stdbool.h>
#include <stdlib.h>

typedef struct linked_block{
    chblix_t next_block;
    chblix_t prev_block;
    chblix_t chblix;
    char flag;
    int64_t mem_start;
} linked_block_t;

typedef struct ptr_chblix{
    chunk_t* chunk;
    int64_t block_idx;
} ptr_chblix_t;

typedef enum {LB_SUCCESS = 0, LB_FAIL = -1} linked_block_status_t;
typedef enum {LB_FREE = 0, LB_USED = 1} linked_block_flag_t;

#define lb_for_each(chunk, chblix, ppl) \
    chunk_t* chunk = ppl_load_chunk(ppl->head); \
    for(chblix_t chblix = lb_pool_start(ppl, &chunk); \
        lb_valid(ppl,chunk, chblix); \
        ++chblix.block_idx,  chblix = lb_nearest_valid_chblix(ppl, chblix, &chunk))

/**
 * \brief       Loads linked block
 * \param[in]   page_pool_ptr: pointer page pool
 * \param[in]   chunk_ptr: pointer to chunk
 * \param[in]   chblix_ptr: pointer to chunk Block Index
 * \param[out]  lb_ptr: Linked Block pointer
 * \return      PPL_SUCCESS on success, PPL_FAIL otherwise
 */

int lb_load_nova_pppp(page_pool_t* ppl, chunk_t* chunk, chblix_t* chblix, linked_block_t* linked_block);
int lb_load_nova_ppp(page_pool_t* ppl, chblix_t* chblix, linked_block_t* linked_block);

chblix_t lb_alloc_m(page_pool_t* page_pool, int64_t mem_start);
chblix_t lb_alloc(page_pool_t* page_pool);
int lb_load(int64_t page_pool_index, const chblix_t* chblix, linked_block_t* lb);
int lb_update_nova(page_pool_t* ppl, const chblix_t* chblix, linked_block_t* lb);
int lb_update(int64_t ppidx, const chblix_t* chblix, linked_block_t* lb);
int lb_dealloc_nova(page_pool_t* ppl, linked_block_t* lb);
int lb_dealloc(int64_t ppidx, chblix_t* chblix);
chblix_t lb_get_next_nova(page_pool_t* ppl, const chblix_t* chblix);
chblix_t lb_go_to_nova(page_pool_t* ppl,
                       chblix_t* chblix,
                       int64_t current_block_idx,
                       int64_t block_idx);
int lb_write(page_pool_t* ppl,
             chblix_t* chblix,
             void *src,
             int64_t size,
             int64_t src_offset);
int lb_read_nova(page_pool_t* ppl,
                 chunk_t* chunk,
                 chblix_t* chblix,
                 void *dest,
                 int64_t size,
                 int64_t src_offset);
int lb_read_nova_5(page_pool_t* ppl,
                   chblix_t* chblix,
                   void *dest,
                   int64_t size,
                   int64_t src_offset);
int lb_read(int64_t pplidx,
            chblix_t* chblix,
            void *dest,
            int64_t size,
            int64_t src_offset);
int64_t lb_useful_space_size(int64_t ppidx, chblix_t* chblix);
int64_t lb_ppl_init(int64_t block_size);
page_pool_t* lb_ppl_load(int64_t ppidx);
chblix_t lb_nearest_valid_chblix(page_pool_t* ppl, chblix_t chblix, chunk_t** chunk);
chblix_t lb_pool_start(page_pool_t* ppl, chunk_t** chunk);
#define lb_ppl_destroy(ppidx) ppl_destroy(ppidx)
bool lb_valid(page_pool_t* ppl, chunk_t* chunk, chblix_t chblix);
int64_t lb_print_used(page_pool_t* ppl);
