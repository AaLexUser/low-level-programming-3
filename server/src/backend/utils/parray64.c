#include "parray64.h"
#include "utils/logger.h"
#include <stdbool.h>
#include <stdio.h>

/**
 * @brief       Initializes parray64
 * @param[in]   block_size: size of block
 * @param[in]   inval: invalid value used when deleting blocks
 * @return      page_index or PA_FAIL
 */

int64_t pa_init64(int64_t block_size, int64_t inval){
    int64_t pa = pa_init(block_size);
    if(pa == PA_FAIL){
        return PA_FAIL;
    }
    parray64_t* parray64 = (parray64_t*)pa_load(pa);
    parray64->inval = inval;
    parray64->parray.lp.mem_start = sizeof(parray64_t);
    return pa;
}


/**
 * @brief       Write to parray64
 * @param[in]   parray: parray64 to write to
 * @param[in]   block_idx: block index to write to
 * @param[in]   value: value to write
 * @return      PA_SUCCESS on success, PA_FAIL otherwise
 */

int pa_write64(parray64_t* parray, int64_t block_idx, int64_t value){
    return pa_write((parray_t*)parray, block_idx, &value, sizeof(int64_t), 0);
}

/**
 * @brief       Read from parray64
 * @param[in]   parray64: parray64 to read from
 * @param[in]   block_idx: block index to read from
 * @param[in]   dest: destination
 * @return      PA_SUCCESS on success, PA_FAIL otherwise
 */

int pa_read64(parray64_t* parray64, int64_t block_idx, int64_t* dest){
    return pa_read((parray_t*)parray64, block_idx, dest, sizeof(int64_t), 0);
}

/**
 * @brief       Delete data from parray64
 * @param[in]   page_index: page index of PArray
 * @param[in]   block_idx: block index to delete
 * @return      PA_SUCCESS on success, PA_FAIL otherwise
 */

int pa_delete64(int64_t page_index, int64_t block_idx){
    parray64_t *pa = (parray64_t *) lp_load(page_index);

    if (!pa) {
        logger(LL_ERROR, __func__, "Unable to load page");
        return PA_FAIL;
    }

    if (pa->parray.size <= block_idx) {
        logger(LL_ERROR, __func__, "Unable to delete from PArray");
        return PA_FAIL;
    }

    if (pa_write64(pa, block_idx, pa->inval) == PA_FAIL){
        logger(LL_ERROR, __func__, "Unable to delete from PArray");
        return PA_FAIL;
    }
    pa->parray.size = block_idx == (pa->parray.size - 1) ? pa->parray.size-1 : pa->parray.size;
    return PA_SUCCESS;
}

/**
 * @brief       Append value to parray64
 * @param[in]   paidx: page index of PArray
 * @return      page_index or PA_FAIL
 */

int pa_append64(int64_t paidx, int64_t value){
    return pa_append(paidx, &value, sizeof(int64_t));
}

/**
 * @brief       Pop value from parray64
 * @param[in]   paidx: page index of PArray
 * @param[out]  dest: destination
 * @return      PA_SUCCESS on success, PA_EMPY if parray64 empty, PA_FAIL otherwise
 */

int pa_pop64(int64_t paidx, int64_t* dest){
    parray64_t pa = *(parray64_t *) lp_load(paidx);
    do {
        int res = pa_pop(paidx, dest, sizeof(int64_t));
        if (res == PA_FAIL) {
            logger(LL_ERROR, __func__, "Unable to pop from PArray");
            return PA_FAIL;
        }
        if(res == PA_EMPTY){
            return PA_EMPTY;
        }
        if (*dest == pa.inval) {
            pa_delete64(paidx, pa.parray.size - 1);
        }
    } while (*dest == pa.inval);
    return PA_SUCCESS;
}

/**
 * @brief           Returns block index of first occurence of value in PArray
 * @param[in]       paidx: page index of parray64
 * @param[in]       value: value to find
 * @return          block index of first occurence of value in PArray or PA_FAIL
 */

int64_t pa_find_first_int64(int64_t paidx, int64_t value){
    int64_t size = pa_size(paidx);

    /* If PArray is empty */
    if(size == 0){
        return PA_FAIL;
    }

    if(size == PA_FAIL){
        logger(LL_ERROR, __func__, "Unable to get size of PArray");
        return PA_FAIL;
    }

    /* Load PArray */
    parray64_t *pa = (parray64_t *) lp_load(paidx);

    /* Reading all blocks */
    int64_t blocks[(size_t) size];
    if(pa_read_blocks(paidx, 0, &blocks, size * pa->parray.block_size, 0) == PA_FAIL){
        logger(LL_ERROR, __func__, "Unable to read PArray");
        return PA_FAIL;
    }

    for(int64_t i = 0; i < size; i++){
        int64_t val = blocks[i];
        if(val == value){
            return i;
        }
    }
    return PA_FAIL;
}

/**
 * @brief           Check if value exists in parray64
 * @param[in]       paidx: page index of parray64
 * @param[in]       value: value to find
 * @return          true if value exists, false if not, PA_FAIL on error
 */

int pa_exists64(int64_t paidx, int64_t value){
    int64_t size = pa_size(paidx);

    /* If PArray is empty */
    if(size == 0){
        return false;
    }

    if(size == PA_FAIL){
        logger(LL_ERROR, __func__, "Unable to get size of PArray");
        return PA_FAIL;
    }

    /* Load PArray */
    parray64_t *pa = (parray64_t *) lp_load(paidx);

    /* Reading all blocks */
    int64_t blocks[(size_t) size];
    if(pa_read_blocks(paidx, 0, &blocks, size * pa->parray.block_size, 0) == PA_FAIL){
        logger(LL_ERROR, __func__, "Unable to read PArray");
        return PA_FAIL;
    }

    for(int64_t i = 0; i < size; i++){
        int64_t val = blocks[i];
        if(val == value){
            return true;
        }
    }
    return false;
}

/**
 * @brief       Push unique int64_t to parray64
 * @param[in]   paidx: page index of PArray
 * @param[in]   value: value to push
 * @return      PA_SUCCESS or PA_FAIL
 */

int pa_push_unique64(int64_t paidx, int64_t value){
    int res = pa_exists64(paidx, value);
    if(res == PA_FAIL){
        logger(LL_ERROR, __func__, "Unable to check if value exists");
        return PA_FAIL;
    }
    else if(res == false) {
        if(pa_append64(paidx, value) != PA_SUCCESS){
            logger(LL_ERROR, __func__, "Unable to append value");
            return PA_FAIL;
        }
    }
    return PA_SUCCESS;
}

/**
 * @brief           Deletes first occurences of value in PArray
 * @param[in]       paidx: page index of PArray
 * @param[in]       value: value to delete
 * @return          PA_SUCCESS on success or not found, PA_FAIL otherwise
 */

int pa_delete_unique64(int64_t paidx, int64_t value){
    int64_t block_index = pa_find_first_int64(paidx, value);
    if(block_index == PA_FAIL){
        logger(LL_INFO, __func__, "Value %ld not found in PArray %ld", value, paidx);
        return PA_SUCCESS;
    }
    if(pa_delete64(paidx, block_index) != PA_SUCCESS){
        logger(LL_ERROR, __func__, "Unable to delete value");
        return PA_FAIL;
    }
    return PA_SUCCESS;
}



