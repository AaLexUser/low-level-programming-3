#pragma once

#include "core/io/linked_pages.h"
#include <stdint.h>


typedef struct parray{
    linked_page_t lp;
    int64_t page_idx;
    int64_t size;
    int64_t block_size;
} parray_t;

enum {PA_SUCCESS = 0, PA_FAIL = -1, PA_EMPTY = -2};

int64_t pa_init(int64_t block_size);
parray_t* pa_load(int64_t page_index);
int pa_destroy(int64_t page_index);
int pa_write(parray_t *pa, int64_t block_idx, void *src, int64_t size, int64_t src_offset);
int pa_read(parray_t* parray, int64_t block_idx, void *dest, int64_t size, int64_t src_offset);
int pa_read_blocks(int64_t paidx, int64_t stblidx, void *dest, int64_t size, int64_t src_offset);
int pa_append(int64_t paidx, void *src, int64_t size);
int pa_pop(int64_t pa_index, void *dest, int64_t size);
int64_t pa_size(int64_t page_index);
int64_t pa_block_size(int64_t page_index);
int pa_at(int64_t page_index, int64_t block_idx, void *dest);
