#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#if defined(_WIN32)
#define PAGE_SIZE 65536
#include <windows.h>
typedef struct file {
	char *filename;
	HANDLE h_file;
    HANDLE h_map;
	void *cur_mmaped_data;
	off_t cur_page_offset;
    off_t file_size;
    int64_t max_page_index;
} file_t;
#endif

#if defined(__linux__) || defined(__APPLE__) || defined(__unix__)
#include <unistd.h>
#ifndef PAGE_SIZE
#define PAGE_SIZE sysconf(_SC_PAGESIZE)
#endif

typedef struct file {
    char *filename;
    int fd;
    void *cur_mmaped_data;
    off_t cur_page_offset;
    off_t file_size;
    int64_t max_page_index;
} file_t;
#endif

enum {FILE_FAIL=-1, FILE_SUCCESS=0};

off_t fl_cur_page_offset(file_t* file);
void* fl_cur_mmaped_data(file_t* file);
off_t fl_file_size(file_t* file);
uint64_t fl_number_pages(file_t* file);
uint64_t fl_page_index(off_t page_offset);
off_t fl_page_offset(uint64_t page_index);
int64_t fl_current_page_index(file_t* file);


int init_file(const char* file_name, file_t* file);
int close_file(file_t* file);
int delete_file(file_t* file);
int mmap_page(off_t offset, file_t* file);
int map_page_on_addr(off_t offset, file_t* file, void* addr);
int sync_page(void* mmaped_data);
int unmap_page(void** mmaped_data, file_t* file);
int init_page(file_t* file);
int delete_last_page(file_t* file);
int write_page(file_t* file, void* src, uint64_t size, off_t offset);
int read_page(file_t* file, void* dest, uint64_t size, off_t offset);
#endif
