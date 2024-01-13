#include "../src/test.h"
#include "core/io/file.h"
#include <stdio.h>

DEFINE_TEST(write_and_read){
    file_t* file = malloc(sizeof(file_t));
    assert(init_file("test.db", file) == FILE_SUCCESS);
    if(fl_file_size(file) > 0){
        assert(delete_file(file) == FILE_SUCCESS);
        assert(init_file("test.db", file) == FILE_SUCCESS);
    }
    char str[] = "12345678";
    init_page(file);
    off_t page_offset = file->cur_page_offset;
    write_page(file, str, sizeof(str), 0);
    unmap_page(&file->cur_mmaped_data, file);
    mmap_page(page_offset, file);
    char* read_str = malloc(sizeof(str));
    read_page(file, read_str, sizeof(str), 0);
    assert(strcmp(str, read_str) == 0);
    free(read_str);
    unmap_page(&file->cur_mmaped_data, file);
    delete_file(file);
    free(file);
}

DEFINE_TEST(two_write){
    file_t* file = malloc(sizeof(file_t));
    assert(init_file("test.db", file) == FILE_SUCCESS);
    if(fl_file_size(file) > 0){
        assert(delete_file(file) == FILE_SUCCESS);
        assert(init_file("test.db", file) == FILE_SUCCESS);
    }
    char str1[] = "12345678";
    init_page(file);
    off_t page_offset = file->cur_page_offset;
    write_page(file, str1, sizeof(str1), 0);
    unmap_page(&file->cur_mmaped_data, file);

    mmap_page(page_offset, file);
    char str2[] = "abcdefg";
    write_page(file, str2, sizeof(str2), sizeof(str1));
    char* read_str2 = malloc(sizeof(str2));
    read_page(file, read_str2,sizeof(str2), sizeof(str1));
    assert(strcmp(str2, read_str2) == 0);
    unmap_page(&file->cur_mmaped_data, file);

    delete_file(file);
    free(read_str2);
    free(file);
}
DEFINE_TEST(two_pages){
    file_t* file = malloc(sizeof(file_t));
    assert(init_file("test.db", file) == FILE_SUCCESS);
    if(fl_file_size(file) > 0){
        assert(delete_file(file) == FILE_SUCCESS);
        assert(init_file("test.db", file) == FILE_SUCCESS);
    }
    char str1[] = "12345678";
    init_page(file);
    off_t page1_offset = file->cur_page_offset;
    write_page(file, str1, sizeof(str1), 0);
    unmap_page(&file->cur_mmaped_data, file);

    init_page(file);
    off_t page2_offset = file->cur_page_offset;
    char str2[] = "abcdefg";
    write_page(file, str2, sizeof(str2), 0);
    unmap_page(&file->cur_mmaped_data, file);

    mmap_page(page1_offset, file);
    char* read_str1 = malloc(sizeof(str1));
    read_page(file, read_str1,sizeof(str1), 0);
    assert(strcmp(str1, read_str1) == 0);
    unmap_page(&file->cur_mmaped_data, file);

    mmap_page(page2_offset, file);
    char* read_str2 = malloc(sizeof(str2));
    read_page(file, read_str2,sizeof(str2), 0);
    assert(strcmp(str2, read_str2) == 0);
    unmap_page(&file->cur_mmaped_data, file);

    free(read_str1);
    free(read_str2);

    delete_file(file);
    free(file);
}

DEFINE_TEST(delete_last_page){
    file_t* file = malloc(sizeof(file_t));
    assert(init_file("test.db", file) == FILE_SUCCESS);
    if(fl_file_size(file) > 0){
        assert(delete_file(file) == FILE_SUCCESS);
        assert(init_file("test.db", file) == FILE_SUCCESS);
    }
    char str1[] = "12345678";
    init_page(file);
    off_t page_offset1 = file->cur_page_offset;
    write_page(file, str1, sizeof(str1), 0);
    unmap_page(&file->cur_mmaped_data, file);

    init_page(file);
    off_t page_offset2 = file->cur_page_offset;
    char str2[] = "abcdefg";
    write_page(file, str2, sizeof(str2), 0);
    unmap_page(&file->cur_mmaped_data, file);

    mmap_page(page_offset1, file);
    char* read_str1 = malloc(sizeof(str1));
    read_page(file, read_str1,sizeof(str1), 0);
    assert(strcmp(str1, read_str1) == 0);
    unmap_page(&file->cur_mmaped_data, file);

    mmap_page(page_offset2, file);
    char* read_str2 = malloc(sizeof(str2));
    read_page(file, read_str2,sizeof(str2), 0);
    assert(strcmp(str2, read_str2) == 0);
    assert(unmap_page(&file->cur_mmaped_data, file) == 0);

    off_t file_size = fl_file_size(file);
    assert(delete_last_page(file) == 0);
    assert(file_size -  PAGE_SIZE == (int) fl_file_size(file));
    assert(delete_file(file) == 0);

    free(read_str1);
    free(read_str2);
    free(file);
}

int main(){
    RUN_SINGLE_TEST(write_and_read);
    RUN_SINGLE_TEST(two_write);
    RUN_SINGLE_TEST(two_pages);
    RUN_SINGLE_TEST(delete_last_page);
}