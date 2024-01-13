#include "pager.h"
#include "backend/utils/parray64.h"
#include "caching.h"
#include "utils/logger.h"

#ifndef PAGER
pager_t* pg_pager;
#define PAGER pg_pager
#endif

#ifndef DELETED_PAGES_START_INDEX
#define DELETED_PAGES_START_INDEX 0
#endif

/**
 * @brief       Creates pager
 * @return      PAGER_SUCCESS on success, PAGER_FAIL otherwise
 */
static int pg_create(void){
    logger(LL_DEBUG, __func__, "Creating pager");
    PAGER->deleted_pages = -1;
    PAGER->deleted_pages = pa_init64(sizeof(int64_t), -1);
    return PAGER_SUCCESS;
}

/**
 * @breif       Initializes pager
 * @param[in]   file_name: name of file to store data
 * @return      PAGE_SUCCESS on success, PAGE_FAIL otherwise
 */

int pg_init(const char* file_name){
    logger(LL_DEBUG, __func__, "Initializing pager");
    PAGER = malloc(sizeof(pager_t));
    if (ch_init(file_name, &PAGER->ch) == CH_FAIL) {
        logger(LL_ERROR, __func__, "Unable to initialize caching");
        return PAGER_FAIL;
    }
    if(pg_max_page_index()){
        pg_create();
    }
    else{
        PAGER->deleted_pages = 0;
    }
    return PAGER_SUCCESS;
}


/**
 * @brief       Delete file and destroy caching
 * @return      PAGE_SUCCESS on success, PAGE_FAIL otherwise
 */

int pg_delete(void){
    logger(LL_DEBUG, __func__, "Deleting file");
    if(ch_delete(&PAGER->ch) == CH_FAIL){
        logger(LL_ERROR, __func__, "Unable to delete caching");
        return PAGER_FAIL;
    }
    free(PAGER);
    return PAGER_SUCCESS;
}

/**
 * @brief       Close file and destroy caching
 * @return      PAGE_SUCCESS on success, PAGE_FAIL otherwise
 */

int pg_close(void){
    logger(LL_DEBUG, __func__, "Closing file");
    if(ch_close(&PAGER->ch) == CH_FAIL){
        logger(LL_ERROR, __func__, "Unable to delete caching");
        return PAGER_FAIL;
    }
    free(PAGER);
    return PAGER_SUCCESS;
}
/**
 * Allocates page
 * @brief Loads free pages from file or allocates new page
 * @return index of page or PAGER_FAIL
 */

int64_t pg_alloc(void){
    logger(LL_DEBUG, __func__, "Allocating page");
    int64_t page_idx = -1;

    int64_t del_pag_idx = -1;

    if(PAGER->deleted_pages != -1){
        if((pa_pop64(PAGER->deleted_pages, &del_pag_idx)) == PA_SUCCESS){
            page_idx = del_pag_idx;
        }
        if(del_pag_idx != -1){
            ch_use_again(&PAGER->ch, del_pag_idx);
        }
    }

    if(del_pag_idx == -1 || del_pag_idx > pg_max_page_index()){
        logger(LL_DEBUG, __func__, "Unable to pop page from deleted pages, allocating new page");
        if((page_idx = ch_new_page(&PAGER->ch)) == CH_FAIL){
            logger(LL_ERROR, __func__, "Unable to load new page");
            return PAGER_FAIL;
        }
    }
    return page_idx;
}


/**
 * Deallocates page
 * @warning double deleting same page may cause program crash in future
 * @brief Deallocates page
 * @param page_index
 * @return PAGER_SUCCESS or PAGER_FAIL
 */

int pg_dealloc(int64_t page_index) {
    logger(LL_DEBUG, __func__, "Deallocating page %ld", page_index);
    pa_push_unique64(PAGER->deleted_pages, page_index);
    ch_delete_page(&PAGER->ch, page_index);
    return PAGER_SUCCESS;
}

int pg_rm_cached(int64_t page_index){
    ch_remove(&PAGER->ch, page_index);
    return PAGER_SUCCESS;
}

/**
 * @brief       Loads page
 * @param[in]   page_index: index of page
 * @return      pointer to page or NULL
 */

void* pg_load_page(int64_t page_index) {
    logger(LL_DEBUG, __func__, "Loading page %ld", page_index);
    void* page_ptr = NULL;
    int res = ch_load_page(&PAGER->ch, page_index, &page_ptr);
    if (res == CH_FAIL) {
        logger(LL_ERROR, __func__, "Unable to load page %ld", page_index);
        return NULL;
    }
    if (res == CH_DELETED) {
        logger(LL_ERROR, __func__, "Requested deleted page: %ld", page_index);
        return NULL;
    }
    return page_ptr;
}

/**
 * @brief       Write to page
 * @param[in]   page_index: page index
 * @param[in]   src: source
 * @param[in]   size: size to write
 * @param[in]   offset: offset in page to write to
 * @return      PAGER_SUCCESS on success, PAGER_FAIL otherwise
 */

int pg_write(int64_t page_index, void* src, size_t size, off_t offset){
    logger(LL_DEBUG, __func__,
           "Writing to page, page index: %ld, src: %p, size: %ld, offset: %ld",
           page_index, src, size, offset);
    int res = ch_write(&PAGER->ch, page_index, src, size, offset);
    if(res == CH_FAIL){
        logger(LL_ERROR, __func__,
               "Unable to write to page, page index: %ld, src: %p, size: %ld, offset: %ld",
               page_index, src, size, offset);
        return PAGER_FAIL;
    }
    return PAGER_SUCCESS;
}

/**
 * @brief       Read from page
 * @param[in]   page_index: page index
 * @param[out]  dest: destination
 * @param[in]   size: size to read
 * @param[in]   offset: offset in page to read from
 * @return      PAGER_SUCCESS on success, PAGER_FAIL otherwise
 */

int pg_copy_read(int64_t page_index, void* dest, size_t size, off_t offset){
    logger(LL_DEBUG, __func__, "Reading from page");
    if(ch_copy_read(&PAGER->ch, page_index, dest, size, offset) == CH_FAIL){
        logger(LL_ERROR, __func__, "Unable to read from page");
        return PAGER_FAIL;
    }
    return PAGER_SUCCESS;
}

/**
 * @brief   Get current file size
 * @return  file size
 */

off_t pg_file_size(void){
    return ch_file_size(&PAGER->ch);
}

#define $pg_max_page_index() (PAGER->ch.file.max_page_index)

/**
 * @brief   Get current max page index
 * @return  max page index
 */
 int64_t pg_max_page_index(void){
     return $pg_max_page_index();
 }
/**
 * @brief   Get current cached size
 * @return  cached size
 */
size_t pg_cached_size(void){
    return ch_size(&PAGER->ch);
 }


