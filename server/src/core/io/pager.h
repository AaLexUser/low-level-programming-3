#pragma once
#include "caching.h"
#include <stdint.h>
#include <stdlib.h>

#ifndef PAGE_POOL_SIZE
#define PAGE_POOL_SIZE 100
#endif

typedef struct pager{
    caching_t ch;
    int64_t deleted_pages; // index of parray page with deleted pages
} pager_t;

enum PagerStatuses{PAGER_SUCCESS = 0, PAGER_FAIL = -1, PAGER_DELETED=-2};


int pg_init(const char* file_name);
int pg_delete(void);
int pg_close(void);
int64_t pg_alloc(void);
int pg_dealloc(int64_t page_index);
int pg_rm_cached(int64_t page_index);
void* pg_load_page(int64_t page_index);
int pg_write(int64_t page_index, void* src, size_t size, off_t offset);
int pg_copy_read(int64_t page_index, void* dest, size_t size, off_t offset);
off_t pg_file_size(void);
int64_t pg_max_page_index(void);
size_t pg_cached_size(void);


