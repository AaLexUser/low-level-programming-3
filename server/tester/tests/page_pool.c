#include "../src/test.h"
#include "core/io/caching.h"
#include "core/page_pool/page_pool.h"
#include "core/io/pager.h"
#include "utils/logger.h"
#include <stdio.h>

DEFINE_TEST(write_and_read){
    assert(pg_init("test.db") == PAGER_SUCCESS);
    char str[] = "12345678";
    int64_t ppidx = ppl_init(9);
    chblix_t block1 = ppl_alloc(ppidx);
    ppl_write_block(ppidx, &block1, str, sizeof(str), 0);
    char* read_str = malloc(sizeof(str));
    ppl_read_block(ppidx, &block1, read_str, sizeof(str), 0);
    assert(strcmp(str, read_str) == 0);
    free(read_str);
    pg_delete();
}

DEFINE_TEST(several_write){
    assert(pg_init("test.db") == PAGER_SUCCESS);
    char str[] = "12345678";
    int64_t block_size = 9;
    int64_t ppidx = ppl_init(block_size);
    int64_t count = (int64_t)(PAGE_SIZE - sizeof_Page_Header) / block_size;
    chblix_t blocks[count];
    for(int64_t i = 0; i < count; i++){
        chblix_t block = ppl_alloc(ppidx);
        blocks[i] = block;
        ppl_write_block(ppidx, &block, str, sizeof(str), 0);
    }
    for(int64_t i = 0; i < count; i++){
        chblix_t block = blocks[i];
        char* read_str = malloc(sizeof(str));
        ppl_read_block(ppidx, &block, read_str, sizeof(str), 0);
        assert(strcmp(str, read_str) == 0);
        free(read_str);
    }
    pg_delete();
}

DEFINE_TEST(close_and_open){
    assert(pg_init("test.db") == PAGER_SUCCESS);
    char str[] = "12345678";
    int64_t block_size = 9;
    int64_t ppidx = ppl_init(block_size);
    int64_t count = (int64_t)(PAGE_SIZE - sizeof_Page_Header) / block_size;
    count--; // to prevent page_pool expand
    chblix_t blocks[count];
    for(int64_t i = 0; i < count ; i++){
        chblix_t block = ppl_alloc(ppidx);
        blocks[i] = block;
        ppl_write_block(ppidx, &block, str, sizeof(str), 0);
    }
    pg_close();
    assert(pg_init("test.db") == PAGER_SUCCESS);
    for(int64_t i = 0; i < count; i++){
        chblix_t block = blocks[i];
        char* read_str = malloc(sizeof(str));
        ppl_read_block(ppidx, &block, read_str, sizeof(str), 0);
        assert(strcmp(str, read_str) == 0);
        free(read_str);
    }
    pg_delete();
}

DEFINE_TEST(dealloc){
    assert(pg_init("test.db") == PAGER_SUCCESS);
    char str[] = "12345678";
    int64_t block_size = 9;
    int64_t ppidx = ppl_init(block_size);
    page_pool_t* ppl = ppl_load(ppidx);
    linked_page_t* linkedPage = lp_load(ppl->current_idx);
    int64_t count = lp_useful_space_size(linkedPage) / block_size;
    chblix_t blocks[3 * count];
    for(int64_t i = 0; i < 3 * count ; i++){
        chblix_t block = ppl_alloc(ppidx);
        blocks[i] = block;
        ppl_write_block(ppidx, &block, str, sizeof(str), 0);
    }
    for(int64_t i = count; i < 2 * count ; i++){
        chblix_t block = blocks[i];
        ppl_dealloc(ppidx, &block);
    }
    int64_t page = pg_alloc();
    assert(page == blocks[count].chunk_idx);

    pg_delete();

}

DEFINE_TEST(ultra_wide_page){
    assert(pg_init("test.db") == PAGER_SUCCESS);
    char str[] = "12345678";
    int64_t block_size = 2 * PAGE_SIZE;
    int64_t ppidx = ppl_init(block_size);
    int64_t count = 2;
    chblix_t blocks[count];
    for(int64_t i = 0; i < count; i++){
        chblix_t block = ppl_alloc(ppidx);
        blocks[i] = block;
        ppl_write_block(ppidx, &block, str, sizeof(str), PAGE_SIZE+1000);
    }
    for(int64_t i = 0; i < count; i++){
        chblix_t block = blocks[i];
        char* read_str = malloc(sizeof(str));
        ppl_read_block(ppidx, &block, read_str, sizeof(str), PAGE_SIZE+1000);
        assert(strcmp(str, read_str) == 0);
        free(read_str);
    }
    pg_delete();
}

int main(){
    RUN_SINGLE_TEST(write_and_read);
    RUN_SINGLE_TEST(several_write);
    RUN_SINGLE_TEST(close_and_open);
    RUN_SINGLE_TEST(dealloc);
    RUN_SINGLE_TEST(ultra_wide_page);
}