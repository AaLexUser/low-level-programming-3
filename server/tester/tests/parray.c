#include "../src/test.h"
#include "core/io/caching.h"
#include "core/io/pager.h"
#include "core/io/linked_pages.h"
#include "backend/utils/parray.h"

DEFINE_TEST(write_and_read){
    assert(pg_init("test.db") == PAGER_SUCCESS);
    char str[] = "1234567";
    int64_t array = pa_init(sizeof(str));
    for(int64_t i = 0; i < 1000; i++){
        pa_append(array, str, sizeof(str));
    }
    for(int64_t i = 0; i < 1000; i++){
        char* read_str = malloc(sizeof(str));
        pa_at(array, i, read_str);
        assert(strcmp(str, read_str) == 0);
        free(read_str);
    }
    pa_destroy(array);
    pg_delete();
}

DEFINE_TEST(close_and_open){
    assert(pg_init("test.db") == PAGER_SUCCESS);
    char str[] = "1234567";
    int64_t array = pa_init(sizeof(str));
    for(int64_t i = 0; i < 3000; i++){
        pa_append(array, str, sizeof(str));
    }
    pg_close();

    assert(pg_init("test.db") == PAGER_SUCCESS);
    for(int64_t i = 0; i < 3000; i++){
        char* read_str = malloc(sizeof(str));
        pa_at(array, i, read_str);
        assert(strcmp(str, read_str) == 0);
        free(read_str);
    }
    pa_destroy(array);
    pg_delete();
}



int main(){
    RUN_SINGLE_TEST(write_and_read);
    RUN_SINGLE_TEST(close_and_open);
}