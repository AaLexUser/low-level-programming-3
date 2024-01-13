#include "../src/test.h"
#include "core/io/pager.h"
#include "core/io/caching.h"
#include <stdio.h>

DEFINE_TEST(allocate_deallocate){
    assert(pg_init("test.db") == PAGER_SUCCESS);
    if(pg_file_size() != 0){
        assert(pg_delete() == PAGER_SUCCESS);
        assert(pg_init("test.db") == PAGER_SUCCESS);
    }
    int64_t page1 = pg_alloc();
    assert(page1 == 1);
    int64_t page2 = pg_alloc();
    assert(page2 == 2);
    int64_t page3 = pg_alloc();
    assert(page3 == 3);
    pg_dealloc(page2);
    int64_t page4 = pg_alloc();
    assert(page4 == 2);
    pg_dealloc(page1);
    pg_delete();
}

DEFINE_TEST(double_dealloc){
    assert(pg_init("test.db") == PAGER_SUCCESS);
    if(pg_file_size() != 0){
        assert(pg_delete() == PAGER_SUCCESS);
        assert(pg_init("test.db") == PAGER_SUCCESS);
    }
    int64_t page1 = pg_alloc();
    assert(page1 == 1);
    int64_t page2 = pg_alloc();
    assert(page2 == 2);
    int64_t page3 = pg_alloc();
    assert(page3 == 3);
    pg_dealloc(page2);
    pg_dealloc(page2);
    int64_t page4 = pg_alloc();
    assert(page4 == 2);
    pg_dealloc(page1);
    pg_delete();
}


int main(){
    RUN_SINGLE_TEST(allocate_deallocate);
    RUN_SINGLE_TEST(double_dealloc);
}