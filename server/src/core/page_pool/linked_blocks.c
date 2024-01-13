#include "linked_blocks.h"
#include "core/io/file.h"
#include "core/io/pager.h"
#include "page_pool.h"
#include "utils/logger.h"

#include <math.h>

/**
 * \brief       Allocates new linked block, with custom memory start
 * \param[in]   page_pool: pointer to page pool
 * \param[in]   mem_start: memory start
 * \return      chblix or chblix_fail
 */

chblix_t lb_alloc_m(page_pool_t* page_pool, int64_t mem_start){
    logger(LL_DEBUG, __func__, "Linked_block allocating start.");
    if(page_pool == NULL){
        logger(LL_ERROR, __func__, "Invalid argument: page_pool is NULL");
        return chblix_fail();
    }

    chblix_t chblix = ppl_alloc_nova(page_pool);
    if (chblix.block_idx == -1) {
        logger(LL_ERROR, __func__, "Unable to allocate block");
        return chblix_fail();
    }

    chunk_t* chunk = ppl_load_chunk(chblix.chunk_idx);

    linked_block_t* lb = malloc(page_pool->block_size); /* Don't forget to free it */
    lb_load_nova_pppp(page_pool, chunk,  &chblix, lb);

    lb->next_block = chblix_fail();
    lb->prev_block = chblix_fail();
    lb->chblix = chblix;
    lb->flag = LB_USED;
    lb->mem_start = mem_start;

    lb_update_nova(page_pool, &chblix, lb);

    logger(LL_DEBUG, __func__,
           "Linked_block allocating finished. Linked_block chunk_index: %ld, block_index: %ld",
           chblix.chunk_idx, chblix.block_idx);

    free(lb);
    return chblix;
}


/**
 * \brief       Allocates new linked block
 * \param[in]   page_pool: pointer to page pool
 * \return      chblix or chblix_fail
 */

chblix_t lb_alloc(page_pool_t* page_pool) {
    return lb_alloc_m(page_pool, sizeof(linked_block_t));
}

/**
 * \brief       Loads linked block
 * \param[in]   page_pool_idx: Fist page index of page pool
 * \param[in]   chblix: Chunk Block Index
 * \param[out]  lb: Linked Block pointer
 * \return      LB_SUCCESS on success, LB_FAIL otherwise
 */

int lb_load(int64_t page_pool_index, const chblix_t* chblix, linked_block_t* lb) {
    page_pool_t *ppl = ppl_load(page_pool_index);
    if (ppl == NULL) {
        logger(LL_ERROR, __func__, "Unable to load page pool");
        return LB_FAIL;
    }

    if(ppl_read_block(page_pool_index, chblix, lb, ppl->block_size, 0) != PPL_SUCCESS){
        logger(LL_ERROR, __func__, "Unable to read block");
        return LB_FAIL;
    }

    return LB_SUCCESS;
}

/**
 * @brief       Update linked block
 * @param[in]   ppl: Page pool pointer
 * @param[in]   chblix: Chunk Block Index
 * @param[in]   lb: Linked Block pointer
 * @return      LB_SUCCESS on success, LB_FAIL otherwise
 */

int lb_update_nova(page_pool_t* ppl, const chblix_t* chblix, linked_block_t* lb){
    if(ppl_write_block_nova(ppl, chblix, lb, ppl->block_size, 0) != PPL_SUCCESS){
        logger(LL_ERROR, __func__, "Unable to read block");
        return LB_FAIL;
    }
    return LB_SUCCESS;
}

/**
 * @brief       Update linked block
 * @param[in]   ppidx: Fist page index of page pool
 * @param[in]   chblix: Chunk Block Index
 * @param[in]   lb: Linked Block pointer
 * @return      LB_SUCCESS on success, LB_FAIL otherwise
 */

int lb_update(int64_t ppidx, const chblix_t* chblix, linked_block_t* lb) {
    page_pool_t *ppl = ppl_load(ppidx);
    if (ppl == NULL) {
        logger(LL_ERROR, __func__, "Unable to load page pool");
        return LB_FAIL;
    }

    if(ppl_write_block(ppidx, chblix, lb, ppl->block_size, 0) != PPL_SUCCESS){
        logger(LL_ERROR, __func__, "Unable to read block");
        return LB_FAIL;
    }

    return LB_SUCCESS;
}

int lb_dealloc_nova(page_pool_t* ppl, linked_block_t* lb){
    chblix_t fail = chblix_fail();
    while (chblix_cmp(&lb->next_block, &fail) != 0) {
        chblix_t next_block_idx = lb->next_block;

        /* Setting flag to free */
        lb->flag = LB_FREE;
        lb_update_nova(ppl, &lb->chblix, lb);

        /* Deallocating block */
        if (ppl_dealloc_nova(ppl, &lb->chblix) == PPL_FAIL) {
            logger(LL_ERROR, __func__, "Unable to deallocate block");
            free(lb);
            return LB_FAIL;
        }

        /* Loading next block */
        if (lb_load_nova_ppp(ppl, &next_block_idx, lb) == LB_FAIL) {
            logger(LL_ERROR, __func__, "Unable to read block");
            free(lb);
            return LB_FAIL;
        }

    }

    /* Setting flag to free */
    lb->flag = LB_FREE;
    lb_update_nova(ppl, &lb->chblix, lb);

    /* Deallocating block */
    if (ppl_dealloc_nova(ppl, &lb->chblix) == PPL_FAIL) {
        logger(LL_ERROR, __func__, "Unable to deallocate block");
        free(lb);
        return LB_FAIL;
    }
    return LB_SUCCESS;
}

/**
 * @brief       Deallocates linked block
 * @param[in]   ppidx: Fist page index of page pool
 * @param[in]   chblix: Chunk Block Index
 * @return      LB_SUCCESS on success, LB_FAIL otherwise
 */

int lb_dealloc(int64_t ppidx, chblix_t* chblix){
    page_pool_t* page_pool = lb_ppl_load(ppidx);
    if(page_pool == NULL){
        logger(LL_ERROR, __func__, "Unable to load page pool");
        return LB_FAIL;
    }
    chunk_t *chunk = ppl_load_chunk(chblix->chunk_idx);
    if(chunk == NULL){
        logger(LL_ERROR, __func__, "Unable to load chunk");
        return LB_FAIL;
    }

    linked_block_t* lb = malloc(page_pool->block_size); /* Don't forget to free it */
    if (lb_load_nova_pppp(page_pool,chunk, chblix, lb) == LB_FAIL) {
        logger(LL_ERROR, __func__, "Unable to read block");
        free(lb);
        return LB_FAIL;
    }
    if(lb_dealloc_nova(page_pool, lb) == LB_FAIL){
        logger(LL_ERROR, __func__, "Failed to deallocate row");
        return LB_FAIL;
    }
    free(lb);
    return LB_SUCCESS;
}

/**
 * \brief       Loads next linked block
 * \param[in]   ppl: Page pool pointer
 * \param[in]   chblix: chblix of Linked Block
 * \return      pointer to linked_block_t on success, `NULL` otherwise
 */

chblix_t lb_get_next_nova(page_pool_t* ppl, const chblix_t* chblix){
    if (ppl == NULL) {
        logger(LL_ERROR, __func__, "Unable to load page pool");
        return chblix_fail();
    }
    /* Loading Linked Block */
    linked_block_t* lb = malloc(ppl->block_size); /* Don't forget to free it */
    lb_load_nova_ppp(ppl, (chblix_t*)chblix, lb);

    chblix_t fail = chblix_fail();
    chblix_t next_block_idx = lb->next_block;
    if (chblix_cmp(&next_block_idx, &fail) == 0) {
        /* Allocating new block */
        next_block_idx = lb_alloc(ppl);
        if (next_block_idx.block_idx == -1) {
            logger(LL_ERROR, __func__, "Unable to allocate block");
            free(lb);
            return chblix_fail();
        }
        linked_block_t* next_lb = malloc(ppl->block_size); /* Don't forget to free it */
        lb_load_nova_ppp(ppl, &next_block_idx, next_lb);
        next_lb->prev_block = lb->chblix;
        lb_update_nova(ppl, &next_block_idx, next_lb);
        free(next_lb);
        lb->next_block = next_block_idx;
        lb_update_nova(ppl, chblix, lb);
    }

    free(lb);
    return next_block_idx;

}


/**
 * \brief       Go to block
 * \param[in]   ppl: Page pool pointer
 * \param[in]   chblix: Chunk Block Index
 * \param[in]   current_block_idx: Current block index
 * \param[in]   block_idx: Block index to go to
 * \return      chblix_t on success, `chblix_fail()` otherwise
 */

chblix_t lb_go_to_nova(page_pool_t* ppl,
                       chblix_t* chblix,
                       int64_t current_block_idx,
                       int64_t block_idx){
    int64_t counter = current_block_idx;
    chblix_t res = *chblix;
    /* Go to block */
    while (counter != block_idx) {
        res = lb_get_next_nova(ppl, chblix);
        counter++;
    }

    return res;

}

/**
 * \brief       Loads next linked block
 * \param[in]   page_pool_idx: Fist page index of page pool
 * \param[in]   chblix: chblix of Linked Block
 * \return      pointer to linked_block_t on success, `NULL` otherwise
 */

static chblix_t lb_get_next(int64_t page_pool_index,
                     const chblix_t* chblix){

    /* Loading Page Pool*/
    page_pool_t *ppl = ppl_load(page_pool_index);
    if (ppl == NULL) {
        logger(LL_ERROR, __func__, "Unable to load page pool");
        return chblix_fail();
    }

    /* Loading Linked Block */
    linked_block_t* lb = malloc(ppl->block_size); /* Don't forget to free it */
    lb_load(page_pool_index, chblix, lb);

    chblix_t fail = chblix_fail();
    chblix_t next_block_idx = lb->next_block;
    if (chblix_cmp(&next_block_idx, &fail) == 0) {
        /* Allocating new block */
        next_block_idx = lb_alloc(ppl);
        if (next_block_idx.block_idx == -1) {
            logger(LL_ERROR, __func__, "Unable to allocate block");
            free(lb);
            return chblix_fail();
        }
        linked_block_t* next_lb = malloc(ppl->block_size); /* Don't forget to free it */
        lb_load(page_pool_index, &next_block_idx, next_lb);
        next_lb->prev_block = lb->chblix;
        lb_update(page_pool_index, &next_block_idx, next_lb);
        free(next_lb);
        lb->next_block = next_block_idx;
        lb_update(page_pool_index, chblix, lb);
    }

    free(lb);
    return next_block_idx;
}

/**
 * \brief       Go to block
 * \param[in]   page_pool_idx: Fist page index of page pool
 * \param[in]   chblix: Chunk Block Index
 * \param[in]   current_block_idx: Current block index
 * \param[in]   block_idx: Block index to go to
 * \return      chblix_t on success, `chblix_fail()` otherwise
 */

static chblix_t lb_go_to(int64_t pplidx,
                  chblix_t* chblix,
                  int64_t current_block_idx,
                  int64_t block_idx) {

    int64_t counter = current_block_idx;
    chblix_t res = *chblix;
    /* Go to block */
    while (counter != block_idx) {
        res = lb_get_next(pplidx, chblix);
        counter++;
    }

    return res;
}

/**
 * \brief       Read from linked block
 * \param[in]   ppl: Page Pool pointer
 * \param[in]   chblix: Chunk Block Index
 * \param[in]   src: Source to write
 * \param[in]   size: Size to read
 * \param[in]   src_offset: Offset in block to write
 * \return      LB_SUCCESS on success, LB_FAIL otherwise
 */

int lb_write(page_pool_t* ppl,
                  chblix_t* chblix,
                  void *src,
                  int64_t size,
                  int64_t src_offset){
    /* Check Page Pool*/
    if (ppl == NULL) {
        logger(LL_ERROR, __func__, "Invalid argument, page pool is NULL");
        return LB_FAIL;
    }

    logger(LL_DEBUG, __func__, "Write to Linked Block %ld %ld size: %ld, offset: %ld"
            , chblix->block_idx, chblix->chunk_idx, size, src_offset);

    /* Loading Linked Block */
    linked_block_t* lb = malloc(ppl->block_size); /* Don't forget to free it */
    if (lb_load(page_pool_index(ppl), chblix, lb) == LB_FAIL) {
        logger(LL_ERROR, __func__, "Unable to read block");
        free(lb);
        return LB_FAIL;
    }

    /* Initializing variables */
    int64_t useful_space_size = ppl->block_size - lb->mem_start;
    int64_t start_block = floor((double) src_offset / (double) useful_space_size);
    int64_t start_offset = src_offset % useful_space_size;
    int64_t blocks_needed = ceil((double)(size + start_offset) / (double) useful_space_size);
    int64_t total_size = size;
    int64_t current_block_idx = 0;
    int64_t header_offset =  lb->mem_start;

    /* Go to start block of write and allocate new blocks if needed */
    chblix_t start_point = chblix_fail();
    start_point = lb_go_to(page_pool_index(ppl), chblix, current_block_idx, start_block);

    /* Write to blocks until all data is written */
    while (blocks_needed > 0){
        /* Calculate size to write */
        int64_t size_to_write = total_size > useful_space_size - start_offset
                                ? useful_space_size - start_offset : total_size;

        /* Write to block */
        if (ppl_write_block_nova(ppl, &start_point, src, size_to_write, header_offset + start_offset) == PPL_FAIL) {
            logger(LL_ERROR, __func__, "Unable to write to block");
            free(lb);
            return LB_FAIL;
        }

        /* Update variables */
        blocks_needed--;

        if(blocks_needed > 0){
            total_size -= useful_space_size - start_offset;
            src = (uint8_t*)src + useful_space_size - start_offset;
            start_offset = 0;

            /* Go to next block */
            start_point = lb_get_next(page_pool_index(ppl), &lb->chblix);
            lb_load(page_pool_index(ppl), &start_point, lb);

        }

    }

    free(lb);
    return LB_SUCCESS;
}



/**
 * @brief       Read from linked block
 * @param[in]   ppl: Page Pool pointer
 * @param[in]   chunk: pointer to chunk
 * @param[in]   chblix: Chunk Block Index
 * @param[out]  dest: Destination to read
 * @param[in]   size: Size to read
 * @param[in]   src_offset: Offset in block to read
 * @return      LB_SUCCESS on success, LB_FAIL otherwise
 */

int lb_read_nova(page_pool_t* ppl,
                 chunk_t* chunk,
                 chblix_t* chblix,
                 void *dest,
                 int64_t size,
                 int64_t src_offset){
    logger(LL_DEBUG, __func__, "Reading Linked Block %ld %ld size: %ld, offset: %ld"
            , chblix->block_idx, chblix->chunk_idx, size, src_offset);
    if(!ppl || !chunk->capacity || !chblix){
        logger(LL_ERROR, __func__, "Invalid arguments");
        return LB_FAIL;
    }

    /* Loading Linked Block */
    linked_block_t* lb = malloc(ppl->block_size); /* Don't forget to free it */
    if (lb_load_nova_pppp(ppl, chunk, chblix, lb) == LB_FAIL) {
        logger(LL_ERROR, __func__, "Unable to read block");
        free(lb);
        return LB_FAIL;
    }
    /* Initializing variables */
    int64_t useful_space_size = ppl->block_size - lb->mem_start;
    int64_t start_block = floor((double) src_offset / (double) useful_space_size);
    int64_t start_offset = src_offset % useful_space_size;
    int64_t blocks_needed = ceil((double)(size + start_offset) / (double) useful_space_size);
    int64_t total_size = size;
    int64_t current_block_idx = 0;
    int64_t header_offset = lb->mem_start;

    /* Go to start block of write and allocate new blocks if needed */
    chblix_t start_point = chblix_fail();
    start_point = lb_go_to_nova(ppl, chblix, current_block_idx, start_block);
    linked_page_t* start_chunk = lp_load(start_point.chunk_idx);

    /* Write to blocks until all data is written */
    while (blocks_needed > 0){
        /* Calculate size to read */
        int64_t size_to_read = total_size > useful_space_size - start_offset
                               ? useful_space_size - start_offset : total_size;

        /* Write to block */
        if (ppl_read_block_nova(ppl, start_chunk, &start_point, dest, size_to_read, header_offset + start_offset) == PPL_FAIL) {
            logger(LL_ERROR, __func__, "Unable to write to block");
            free(lb);
            return LB_FAIL;
        }

        /* Update variables */
        blocks_needed--;

        if(blocks_needed > 0){
            total_size -= (useful_space_size - start_offset);
            dest = (uint8_t*)dest + useful_space_size - start_offset;
            start_offset = 0;

            /* Go to next block */
            start_point = lb_get_next_nova(ppl, &lb->chblix);
            lb_load_nova_ppp(ppl, &start_point, lb);

        }

    }
    free(lb);
    return LB_SUCCESS;

}

int lb_read_nova_5(page_pool_t* ppl,
                   chblix_t* chblix,
                   void *dest,
                   int64_t size,
                   int64_t src_offset){
    chunk_t* chunk = ppl_load_chunk(chblix->chunk_idx);
    return lb_read_nova(ppl, chunk, chblix, dest, size, src_offset);

}

/**
 * @brief       Read from linked block
 * @param[in]   pplidx: Fist page index of page pool
 * @param[in]   chblix: Chunk Block Index
 * @param[out]  dest: Destination to read
 * @param[in]   size: Size to read
 * @param[in]   src_offset: Offset in block to read
 * @return      LB_SUCCESS on success, LB_FAIL otherwise
 */

int lb_read(int64_t pplidx,
            chblix_t* chblix,
            void *dest,
            int64_t size,
            int64_t src_offset){

    logger(LL_DEBUG, __func__, "Reading Linked Block %ld %ld size: %ld, offset: %ld"
            , chblix->block_idx, chblix->chunk_idx, size, src_offset);

    /* Loading Page Pool*/
    page_pool_t *ppl = ppl_load(pplidx);
    if (ppl == NULL) {
        logger(LL_ERROR, __func__, "Unable to load page pool");
        return LB_FAIL;
    }

    /* Loading Linked Block */
    linked_block_t* lb = malloc(ppl->block_size); /* Don't forget to free it */
    if (lb_load(pplidx, chblix, lb) == LB_FAIL) {
        logger(LL_ERROR, __func__, "Unable to read block");
        free(lb);
        return LB_FAIL;
    }

    /* Initializing variables */
    int64_t useful_space_size = ppl->block_size - lb->mem_start;
    int64_t start_block = floor((double) src_offset / (double) useful_space_size);
    int64_t start_offset = src_offset % useful_space_size;
    int64_t blocks_needed = ceil((double)(size + start_offset) / (double) useful_space_size);
    int64_t total_size = size;
    int64_t current_block_idx = 0;
    int64_t header_offset = lb->mem_start;

    /* Go to start block of write and allocate new blocks if needed */
    chblix_t start_point = chblix_fail();
    start_point = lb_go_to(pplidx, chblix, current_block_idx, start_block);

    /* Write to blocks until all data is written */
    while (blocks_needed > 0){
        /* Calculate size to read */
        int64_t size_to_read = total_size > useful_space_size - start_offset
                               ? useful_space_size - start_offset : total_size;

        /* Write to block */
        if (ppl_read_block(pplidx, &start_point, dest, size_to_read, header_offset + start_offset) == PPL_FAIL) {
            logger(LL_ERROR, __func__, "Unable to write to block");
            free(lb);
            return LB_FAIL;
        }

        /* Update variables */
        blocks_needed--;

        if(blocks_needed > 0){
            total_size -= (useful_space_size - start_offset);
            dest = (uint8_t*) dest + useful_space_size - start_offset;
            start_offset = 0;

            /* Go to next block */
            start_point = lb_get_next(pplidx, &lb->chblix);
            lb_load(pplidx, &start_point, lb);

        }

    }
    free(lb);
    return LB_SUCCESS;
}




/**
 * @brief       Get useful space size of linked block
 * @param[in]   ppidx: Fist page index of page pool
 * @param[in]   chblix: Chunk Block Index
 * @return      useful space size on success, LB_FAIL otherwise
 */

int64_t lb_useful_space_size(int64_t ppidx, chblix_t* chblix){
    page_pool_t *ppl = ppl_load(ppidx);
    linked_block_t* lb = malloc(ppl->block_size); /* Don't forget to free it */
    if(lb_load(ppidx, chblix, lb) == LB_FAIL){
        logger(LL_ERROR, __func__, "Unable to load linked block");
        free(lb);
        return LB_FAIL;
    }
    int64_t res = PAGE_SIZE - lb->mem_start;
    free(lb);
    return res;
}

/**
 * @brief       Initialize page pool for Linked Blocks
 * @param[in]   block_size: Size of block
 * @return      page pool index on success, LB_FAIL otherwise
 */

int64_t lb_ppl_init(int64_t block_size){
    int64_t ppidx = ppl_init(block_size + (int64_t)sizeof(linked_block_t));
    if(ppidx == PPL_FAIL){
        logger(LL_ERROR, __func__, "Unable to initialize page pool block size: %ld"
               , block_size);
        return LB_FAIL;
    }

    page_pool_t *ppl = ppl_load(ppidx);
    if(ppl == NULL){
        logger(LL_ERROR, __func__, "Unable to load page pool with index: %ld"
               , ppidx);
        return LB_FAIL;
    }
    return ppidx;
}

/**
 * @brief       Load existing page pool for Linked Blocks
 * @param[in]   ppidx: Fist page index of page pool
 * @return      pointer to page pool on success, `NULL` otherwise
 */

page_pool_t* lb_ppl_load(int64_t ppidx){
    return ppl_load(ppidx);
}


/**
 * @brief Finds the nearest valid block to the given block index within a chunk of a page pool.
 *
 * The nearest valid block is determined by searching for the closest block index that is marked as valid
 * and is within the boundaries of the chunk.
 *
 * @param[in]   ppl: The pointer to the page pool.
 * @param[in]   chunk: The pointer to the chunk.
 * @param[in]   block_idx: The block index to find the nearest valid block for.
 * @return The index of the nearest valid block, or -1 if no valid block is found.
 */

static int64_t lb_nearest_valid_block(page_pool_t* ppl, chunk_t* chunk, int64_t block_idx){
    logger(LL_DEBUG, __func__, "Searching for nearest valid block\n"
                            " pool: %ld, block: %ld, chunk: %ld",
        ppl->lp_header.page_index, block_idx, chunk->page_index);
    chblix_t chblix = {.block_idx = block_idx, .chunk_idx = chunk->page_index};
    linked_block_t* temp = malloc(ppl->block_size);
    while (chblix.block_idx != chunk->capacity){
        if(lb_valid(ppl, chunk, chblix)){
            free(temp);
            return chblix.block_idx;
        }
        chblix.block_idx++;
    }

    free(temp);
    return LB_FAIL;
}

/**
 * @brief       Get nearest valid chblix
 * @param[in]   ppl: Page pool pointer
 * @param[in]   chblix: Chunk Block Index
 * @param[in]   current_chunk: pointer to chunk
 * @return      chblix_t on success, `chblix_fail()` otherwise
 */

chblix_t lb_nearest_valid_chblix(page_pool_t* ppl, chblix_t chblix, chunk_t** current_chunk){
    // If there is no chunk, return fail immediately
    if(current_chunk == NULL || *current_chunk == NULL){
        return chblix_fail();
    }

    logger(LL_DEBUG, __func__, "Searching for nearest valid chblix\n"
                            " pool: %ld, chblix block: %ld, chblix chunk: %ld, chunk: %ld",
        ppl->lp_header.page_index, chblix.block_idx, chblix.chunk_idx,
        (*current_chunk)->page_index);

    // Attempt to find a valid chblix until there are no more pages in the chunk
    do {
        // Try to find a valid block in the current chunk
        int64_t result_block = lb_nearest_valid_block(ppl, *current_chunk, chblix.block_idx);

        // If a valid block was found, return it
        if(result_block != LB_FAIL){
            return (chblix_t){.block_idx = result_block, .chunk_idx = (*current_chunk)->page_index};
        }

        // If we've examined all pages and found no valid chblix, return fail
        if((*current_chunk)->next_page == -1){
            return chblix_fail();
        }

        // If there's a next chunk, load it.
        if((*current_chunk)->next_page != -1){
            int64_t chunkid = (*current_chunk)->page_index;
            *current_chunk = ppl_load_chunk((*current_chunk)->next_page);
            chblix.block_idx = 0;
            pg_rm_cached(chunkid);
        }
        if(!(*current_chunk)->capacity){
            logger(LL_ERROR, __func__, "Invalid arguments");
            return chblix_fail();
        }
    } while(*current_chunk != NULL);

    // if error
    logger(LL_ERROR, __func__, "Current chunk is NULL.");
    return chblix_fail();
}

chblix_t lb_pool_start(page_pool_t* ppl, chunk_t** chunk){
    // Check if the pointers are NULL before attempting to access their members.
    if(ppl == NULL || chunk == NULL ){
        logger(LL_ERROR, __func__, "Invalid arguments ppl: %p, chunk: %p", ppl, chunk);
        return chblix_fail();
    }

    logger(LL_DEBUG, __func__, "Searching for pool start\n"
                            " pool: %ld, chunk: %ld",
        ppl->lp_header.page_index, (*chunk)->page_index);

    if(ppl->head == -1){
        return chblix_fail();
    }
    chblix_t start_chblix = {.block_idx = 0, .chunk_idx = (*chunk)->page_index};
    return lb_nearest_valid_chblix(ppl, start_chblix, chunk);
}

bool lb_valid(page_pool_t* ppl, chunk_t* chunk, chblix_t chblix){
    if(chblix_cmp(&chblix, &CHBLIX_FAIL) == 0){
        return false;
    }
    linked_block_t* linked_block = malloc(ppl->block_size);
    if(lb_load_nova_pppp(ppl, chunk,  &chblix, linked_block) == LB_FAIL){
        logger(LL_ERROR, __func__, "Unable to load linked block");
        free(linked_block);
        return false;
    }
    if(linked_block->flag == LB_FREE){
        free(linked_block);
        return false;
    }
    if(chblix_cmp(&linked_block->prev_block, &CHBLIX_FAIL) != 0){
        free(linked_block);
        return false;
    }
    free(linked_block);
    return chblix_cmp(&chblix, &CHBLIX_FAIL) != 0;
}

int lb_load_nova_pppp(page_pool_t* ppl, chunk_t* chunk, chblix_t* chblix, linked_block_t* linked_block){
    return ppl_read_block_nova(ppl,
                               (linked_page_t*)chunk,
                               chblix,
                               linked_block,
                               ppl->block_size,
                               0) == PPL_FAIL ? LB_FAIL : LB_SUCCESS;
}

int lb_load_nova_ppp(page_pool_t* ppl, chblix_t* chblix, linked_block_t* linked_block){
    chunk_t* chunk = ppl_load_chunk(chblix->block_idx);
    return ppl_read_block_nova(ppl,
                               (linked_page_t*)chunk,
                               chblix,
                               linked_block,
                               ppl->block_size,
                               0) == PPL_FAIL ? LB_FAIL : LB_SUCCESS;
}

int64_t lb_print_used(page_pool_t* ppl){
    int64_t count = 0;
    chunk_t* chunk = ppl_load_chunk(ppl->head);
    chblix_t chblix = lb_pool_start(ppl, &chunk);
    int64_t prev_chunk_idx = -1;
    for (;
    chblix_cmp(&chblix, &CHBLIX_FAIL) != 0;
    ++chblix.block_idx, chblix = lb_nearest_valid_chblix(ppl,chblix, &chunk)){
        if(prev_chunk_idx != chunk->page_index){
            printf("\nChunk: %"PRId64"\n", chblix.chunk_idx);
            prev_chunk_idx = chunk->page_index;
        }
        count++;
        printf("%"PRId64"\t", chblix.block_idx);
    }
    printf("\n");
    return count;
}





