#include "../src/test.h"
#include "core/io/caching.h"
#include <stdio.h>

DEFINE_TEST(write_and_read){
    caching_t* caching = malloc(sizeof(caching_t));
    assert(ch_init("test.db", caching) == CH_SUCCESS);
    char str[] = "12345678";
    int64_t page1 = ch_new_page(caching);
    ch_write(caching, page1, str, sizeof(str), 0);
    ch_new_page(caching);
    ch_new_page(caching);
    ch_new_page(caching);
    char* read_str = malloc(sizeof(str));
    ch_copy_read(caching, page1, read_str, sizeof(str), 0);
    assert(strcmp(str, read_str) == 0);
    free(read_str);
    ch_delete(caching);
    free(caching);
}

DEFINE_TEST(two_write){
    caching_t* caching = malloc(sizeof(caching_t));
    assert(ch_init("test.db", caching) == CH_SUCCESS) ;
    char str1[] = "12345678";
    int64_t page1 = ch_new_page(caching);
    ch_write(caching, page1, str1, sizeof(str1), 0);
    ch_new_page(caching);
    ch_new_page(caching);
    ch_new_page(caching);
    char str2[] = "abcdefg";
    ch_write(caching, page1, str2, sizeof(str2), sizeof(str1));

    char* read_str2 = malloc(sizeof(str2));
    ch_copy_read(caching, page1, read_str2, sizeof(str2), sizeof(str1));
    assert(strcmp(str2, read_str2) == 0);
    free(read_str2);

    char* read_str1 = malloc(sizeof(str1));
    ch_copy_read(caching, page1, read_str1, sizeof(str1), 0);
    assert(strcmp(str1, read_str1) == 0);
    free(read_str1);

    ch_delete(caching);
    free(caching);
}
DEFINE_TEST(two_pages){
    caching_t* caching = malloc(sizeof(caching_t));
    assert(ch_init("test.db", caching) == CH_SUCCESS) ;
    char str1[] = "12345678";
    
    int64_t page1 = ch_new_page(caching);
    if(ch_write(caching, page1, str1, sizeof(str1), 0) == -1){
        ch_delete(caching);
        free(caching);
        exit(EXIT_FAILURE);
    }
    ch_new_page(caching);
    ch_new_page(caching);
    ch_new_page(caching);
    char str2[] = "abcdefg";
    int64_t page2 = ch_new_page(caching);
    if(ch_write(caching, page2, str2, sizeof(str2), 0) == -1){
        ch_delete(caching);
        exit(EXIT_FAILURE);
    }
    ch_new_page(caching);
    ch_new_page(caching);
    ch_new_page(caching);
    char* read_str1 = malloc(sizeof(str1));
    ch_copy_read(caching, page1, read_str1, sizeof(str1), 0);
    assert(strcmp(str1, read_str1) == 0);
    free(read_str1);
    char* read_str2 = malloc(sizeof(str2));
    ch_copy_read(caching, page2, read_str2, sizeof(str2), 0);
    assert(strcmp(str2, read_str2) == 0);
    free(read_str2);
    ch_delete(caching);
    free(caching);
}

DEFINE_TEST(read_after_closing){
    caching_t* caching = malloc(sizeof(caching_t));
    assert(ch_init("test.db", caching) == CH_SUCCESS);
    char str1[] = "12345678";
    
    int64_t page1 = ch_new_page(caching);
    if(ch_write(caching, page1, str1, sizeof(str1), 0) == -1){
        ch_delete(caching);
        free(caching);
        exit(EXIT_FAILURE);
    }
    ch_new_page(caching);
    ch_new_page(caching);
    ch_new_page(caching);
    char str2[] = "abcdefg";
    int64_t page2 = ch_new_page(caching);
    if(ch_write(caching, page2, str2, sizeof(str2), 0) == -1){
        ch_delete(caching);
        free(caching);
        exit(EXIT_FAILURE);
    }
    ch_new_page(caching);
    ch_new_page(caching);
    ch_new_page(caching);
    ch_close(caching);

    assert(ch_init("test.db", caching) == CH_SUCCESS);
    char* read_str1 = malloc(sizeof(str1));
    ch_copy_read(caching, page1, read_str1, sizeof(str1), 0);
    assert(strcmp(str1, read_str1) == 0);
    free(read_str1);
    char* read_str2 = malloc(sizeof(str2));
    ch_copy_read(caching, page2, read_str2, sizeof(str2), 0);
    assert(strcmp(str2, read_str2) == 0);
    free(read_str2);
    ch_delete(caching);
    free(caching);
}

DEFINE_TEST(write_after_closing){
    caching_t* caching = malloc(sizeof(caching_t));
    assert(ch_init("test.db", caching) == CH_SUCCESS);
    char str1[] = "12345678";
    int64_t page1 = ch_new_page(caching);
    if(ch_write(caching, page1, str1, sizeof(str1), 0) == -1){
        ch_delete(caching);
        free(caching);
        exit(EXIT_FAILURE);
    }
    ch_new_page(caching);
    ch_new_page(caching);
    ch_new_page(caching);
    char str2[] = "abcdefg";
    int64_t page2 = ch_new_page(caching);
    if(ch_write(caching, page2, str2, sizeof(str2), 0) == -1){
        ch_delete(caching);
        free(caching);
        exit(EXIT_FAILURE);
    }
    ch_new_page(caching);
    ch_new_page(caching);
    ch_new_page(caching);
    ch_close(caching);
    assert(ch_init("test.db", caching) == CH_SUCCESS);
    char str3[] = "12345678";
    ch_write(caching, page1, str3, sizeof(str3), sizeof(str1));
    char* read_str1 = malloc(sizeof(str1));
    ch_copy_read(caching, page1, read_str1, sizeof(str1), 0);
    assert(strcmp(str1, read_str1) == 0);
    free(read_str1);

    char* read_str2 = malloc(sizeof(str2));
    ch_copy_read(caching, page2, read_str2, sizeof(str2), 0);
    assert(strcmp(str2, read_str2) == 0);
    free(read_str2);

    char* read_str3 = malloc(sizeof(str3));
    ch_copy_read(caching, page1, read_str3, sizeof(str3), sizeof(str1));
    assert(strcmp(str3, read_str3) == 0);
    free(read_str3);
    ch_delete(caching);
    free(caching);
}

DEFINE_TEST(delete_last_page){
    caching_t* caching = malloc(sizeof(caching_t));
    assert(ch_init("test.db", caching) == CH_SUCCESS);
    if(ch_file_size(caching) != 0){
        ch_delete(caching);
        assert(ch_init("test.db", caching) == CH_SUCCESS);
    }

    char str1[] = "12345678";
    int64_t page1 = ch_new_page(caching);
    if(ch_write(caching, page1, str1, sizeof(str1), 0) == -1){
        ch_delete(caching);
        exit(EXIT_FAILURE);
    }
    ch_new_page(caching);
    ch_new_page(caching);
    ch_new_page(caching);
    char str2[] = "abcdefg";
    int64_t page2 = ch_new_page(caching);
    if(ch_write(caching, page2, str2, sizeof(str2), 0) == -1){
        ch_delete(caching);
        exit(EXIT_FAILURE);
    }
    ch_new_page(caching);
    ch_new_page(caching);
    ch_new_page(caching);
    assert(ch_file_size(caching) == 8 * PAGE_SIZE);
    assert(ch_delete_page(caching, ch_max_page_index(caching)) == 0);
    assert(ch_file_size(caching) == 7 * PAGE_SIZE);
    assert(ch_delete_page(caching, ch_max_page_index(caching)) == 0);
    assert(ch_file_size(caching) == 6 * PAGE_SIZE);
    assert(ch_delete_page(caching, ch_max_page_index(caching)) == 0);
    assert(ch_file_size(caching) == 5 * PAGE_SIZE);
    char* read_str1 = malloc(sizeof(str1));
    ch_copy_read(caching, page1, read_str1, sizeof(str1), 0);
    assert(strcmp(str1, read_str1) == 0);
    free(read_str1);

    char* read_str2 = malloc(sizeof(str2));
    ch_copy_read(caching, page2, read_str2, sizeof(str2), 0);
    assert(strcmp(str2, read_str2) == 0);
    free(read_str2);
    assert(ch_delete_page(caching, ch_max_page_index(caching)) == 0);
    assert(ch_file_size(caching) == 4 * PAGE_SIZE);
    assert(ch_new_page(caching) == 4);
    ch_delete(caching);
    free(caching);
}




//WARMING: THIS TEST MAY CRASH YOUR IDE, TAKES LONG TIME AND MUCH DISK MEM
DEFINE_TEST(cache_memory_save){
    caching_t* caching = malloc(sizeof(caching_t));
    size_t bignumber = 2097152;
    assert(ch_init("test.db", caching) == CH_SUCCESS);
    char str1[] = "12345678";
    int64_t page1 = ch_new_page(caching);
    if(ch_write(caching, page1, str1, sizeof(str1), 0) == -1){
        ch_delete(caching);
        free(caching);
        exit(EXIT_FAILURE);
    }
    char str[] = "111111111";
    for(size_t i = 0; i < bignumber; i++){
        int64_t page = ch_new_page(caching);
        if(ch_write(caching, page, str, sizeof(str), 0) == -1){
            ch_delete(caching);
            free(caching);
            exit(EXIT_FAILURE);
        }
    }
    char str2[] = "abcdefg";
    int64_t page2 = ch_new_page(caching);
    if(ch_write(caching,page2, str2, sizeof(str2), 0) == -1){
        ch_delete(caching);
        free(caching);
        exit(EXIT_FAILURE);
    }
    for(size_t i = 0; i < bignumber; i++){
        int64_t page = ch_new_page(caching);
        if(ch_write(caching, page, str, sizeof(str), 0) == -1){
            ch_delete(caching);
            free(caching);
            exit(EXIT_FAILURE);
        }
    }
    char* read_str1 = malloc(sizeof(str1));
    ch_copy_read(caching, page1, read_str1, sizeof(str1), 0);
    assert(strcmp(str1, read_str1) == 0);
    char* read_str2 = malloc(sizeof(str2));
    ch_copy_read(caching, page2, read_str2, sizeof(str2), 0);
    assert(strcmp(str2, read_str2) == 0);
    ch_delete(caching);
    free(caching);
}

DEFINE_TEST(unsafe_read){
    caching_t* caching = malloc(sizeof(caching_t));
    assert(ch_init("test.db", caching) == CH_SUCCESS);
    char str1[] = "12345678";
    int64_t page1 = ch_new_page(caching);
    ch_write(caching, page1, str1, sizeof(str1), 0);
    ch_new_page(caching);
    ch_new_page(caching);
    ch_new_page(caching);
    char str2[] = "abcdefg";
    ch_write(caching, page1, str2, sizeof(str2), sizeof(str1));

    char* read_str2 = ch_read(caching, page1, sizeof(str1));
    assert(strcmp(str2, read_str2) == 0);

    char* read_str1 = ch_read(caching, page1, 0);
    assert(strcmp(str1, read_str1) == 0);

    ch_delete(caching);
    free(caching);
}





int main(){
    RUN_SINGLE_TEST(write_and_read);
    RUN_SINGLE_TEST(two_write);
    RUN_SINGLE_TEST(two_pages);
    RUN_SINGLE_TEST(read_after_closing);
    RUN_SINGLE_TEST(write_after_closing);
    RUN_SINGLE_TEST(delete_last_page);
    RUN_SINGLE_TEST(unsafe_read);
//    RUN_SINGLE_TEST(cache_memory_save);
}