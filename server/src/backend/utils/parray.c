#include "parray.h"
#include "utils/logger.h"
#include <stdbool.h>
#include <stdio.h>
#include <sys/types.h>
/**
 * @brief       Initializes PArray
 * @param[in]   block_size: size of block
 * @param[in]   inval: invalid value used when deleting blocks
 * @return      page_index or PA_FAIL
 */

int64_t pa_init(int64_t block_size){
    int64_t page_index = lp_init_m(sizeof(parray_t));
    if(page_index == LP_FAIL){
        logger(LL_ERROR, __func__, "Unable to allocate page");
        return PA_FAIL;
    }
    parray_t *pa = (parray_t *) lp_load(page_index);
    if(!pa){
        logger(LL_ERROR, __func__, "Unable to allocate page");
        return PA_FAIL;
    }
    pa->page_idx = page_index;
    pa->block_size = block_size;
    pa->size = 0;
    return page_index;
}

/**
 * @brief       Loads PArray
 * @param[in]   page_index: page index of parray
 * @return      pointer to parray_t on success, `NULL` otherwise
 */

parray_t* pa_load(int64_t page_index){
    parray_t *pa = (parray_t *) lp_load(page_index);
    if(!pa){
        logger(LL_ERROR, __func__, "Unable to load page");
        return NULL;
    }
    return pa;
}

/**
 * @brif        Destroys PArray
 * @param[in]   page_index: page index of PArray
 * @return      PA_SUCCESS on success, PA_FAIL otherwise
 */

int pa_destroy(int64_t page_index) {
    logger(LL_INFO, __func__, "Destroying PArray");
    if (lp_delete(page_index) == LP_FAIL) {
        logger(LL_ERROR, __func__, "Unable to deallocate page");
        return PA_FAIL;
    }
    return PA_SUCCESS;
}


/**
 * @brief       Write to parray
 * @param[in]   pa: pointer to parray
 * @param[in]   block_idx: block index to write to
 * @param[in]   src: source
 * @param[in]   size: size to write
 * @param[in]   src_offset: offset in block to write to
 * @return      PA_SUCCESS on success, PA_FAIL otherwise
 */

int pa_write(parray_t *pa, int64_t block_idx, void *src, int64_t size, int64_t src_offset){
    if(!pa){
        logger(LL_ERROR, __func__, "Unable to load page");
        return PA_FAIL;
    }
    if(pa->block_size < src_offset + size){
        logger(LL_ERROR, __func__, "Unable to write to PArray");
        return PA_FAIL;
    }
    off_t offset = (off_t)(block_idx * pa->block_size + src_offset);
    if(lp_write(pa->page_idx, src, size, offset) == LP_FAIL){
        logger(LL_ERROR, __func__, "Unable to write to PArray");
        return PA_FAIL;
    }
    pa->size = (block_idx + 1) > pa->size ? block_idx + 1 : pa->size;
    return PA_SUCCESS;
}

/**
 * @brief       Read from PArray
 * @param[in]   parray: parray to read from
 * @param[in]   block_idx: block index to read from
 * @param[out]  dest: destination
 * @param[in]   size: size to read
 * @param[in]   src_offset: offset in block to read from
 * @return      PA_SUCCESS on success, PA_EMPTY if parray empty, PA_FAIL otherwise
 */


int pa_read(parray_t *parray, int64_t block_idx, void *dest, int64_t size, int64_t src_offset) {

    if (!parray) {
        logger(LL_ERROR, __func__, "Parray is NULL");
        return PA_FAIL;
    }

    if (parray->size == 0) {
        logger(LL_INFO, __func__, "Unable to read from empty parray");
        return PA_EMPTY;
    }

    if (parray->block_size < src_offset + size) {
        logger(LL_ERROR, __func__,
               "Unable to read from parray src_size = %ld, src_offset = %ld, block_size = %ld",
               size, src_offset, parray->block_size);
        return PA_FAIL;
    }
    off_t offset = (off_t)(block_idx * parray->block_size + src_offset);
    if (lp_read_copy(parray->page_idx, dest, size, offset) == LP_FAIL) {
        logger(LL_ERROR, __func__, "Unable to read from PArray");
        return PA_FAIL;
    }
    return PA_SUCCESS;
}

/**
 * @brief           Read several blocks at once
 * @param[in]       paidx: page index of PArray
 * @param[in]       stblidx: start block index
 * @param[out]      dest: destination
 * @param[in]       size: size to read
 * @param[in]       src_offset: offset in block to read from
 * @return          PA_SUCCESS or PA_FAIL
 */

int pa_read_blocks(int64_t paidx, int64_t stblidx, void *dest, int64_t size, int64_t src_offset){
    logger(LL_INFO, __func__, "Reading blocks %ld bytes from PArray", size);
    parray_t *pa = (parray_t *) lp_load(paidx);
    if (!pa) {
        logger(LL_ERROR, __func__, "Unable to load page");
        return PA_FAIL;
    }
    off_t offset = (off_t)(stblidx * pa->block_size + src_offset);
    if (lp_read_copy(pa->page_idx, dest, size, offset) == LP_FAIL) {
        logger(LL_ERROR, __func__, "Unable to read from PArray");
        return PA_FAIL;
    }
    return PA_SUCCESS;
}

/**
 * @brif        Append data to PArray
 * @param[in]   paidx: page index of PArray
 * @param[in]   src: source
 * @param[in]   size: size to append
 * @return      PA_SUCCESS on success, PA_FAIL otherwise
 */

int pa_append(int64_t paidx, void *src, int64_t size) {
    parray_t *pa = (parray_t *) lp_load(paidx);

    if (!pa) {
        logger(LL_ERROR, __func__, "Unable to load page");
        return PA_FAIL;
    }
    if(pa_write(pa, pa->size, src, size, 0) == PA_FAIL){
        logger(LL_ERROR, __func__, "Unable to append to parray %ld, pa.size = %ld", paidx
               , pa->size);
        return PA_FAIL;

    }

    return PA_SUCCESS;
}

/**
 * @brief       Pop data from PArray
 * @param[in]   pa_index: page index of PArray
 * @param[out]  dest: destination
 * @param[in]   size: size to pop
 * @return      PA_SUCCESS on success, PA_EMPTY if PArray is empty, PA_FAIL otherwise
 */

int pa_pop(int64_t pa_index, void *dest, int64_t size) {
    parray_t *pa = (parray_t *) lp_load(pa_index);
    int res = pa_read(pa, pa->size - 1, dest, size, 0);
    if(res == PA_SUCCESS){
        pa->size--;
    }
    return res;
}


/**
 * @brief       Get size of PArray
 * @param[in]   page_index
 * @return      size of PArray
 */

int64_t pa_size(int64_t page_index) {
    parray_t *pa = (parray_t *) lp_load(page_index);
    if (!pa) {
        logger(LL_ERROR, __func__, "Unable to load page");
        return PA_FAIL;
    }
    return pa->size;
}

/**
 * Get block size of PArray
 * @param page_index
 * @return  block size of PArray
 */

int64_t pa_block_size(int64_t page_index) {
    parray_t *pa = (parray_t *) lp_load(page_index);
    if (!pa) {
        logger(LL_ERROR, __func__, "Unable to load page");
        return PA_FAIL;
    }
    return pa->block_size;
}

/**
 * Get data from PArray
 * @param page_index
 * @param block_idx
 * @param dest
 * @return  PA_SUCCESS or PA_FAIL
 */

int pa_at(int64_t page_index, int64_t block_idx, void *dest){
    parray_t *pa = (parray_t *) lp_load(page_index);
    if (!pa) {
        logger(LL_ERROR, __func__, "Unable to load page");
        return PA_FAIL;
    }
    if (pa->size <= block_idx) {
        logger(LL_ERROR, __func__, "Unable to read from PArray");
        return PA_FAIL;
    }
    off_t offset = (off_t)(block_idx * pa->block_size);
    if (lp_read_copy(pa->page_idx, dest, pa->block_size, offset) == LP_FAIL) {
        logger(LL_ERROR, __func__, "Unable to read from PArray");
        return PA_FAIL;
    }
    return PA_SUCCESS;
}

